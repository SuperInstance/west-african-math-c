/*
 * palaver.c — Sheaf H⁰ Consensus Dialogue
 *
 * The palaver tree is where communities gather to deliberate and reach
 * consensus. Here we formalize this as computing the global section
 * (H⁰) of a sheaf: each participant holds a local position, and
 * through iterative deliberation, they converge to a consensus —
 * or reveal irreducible disagreement visible as non-zero H¹.
 */

#include "west_african_math.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

static int uf_find(int *parent, int x) {
    while (parent[x] != x) {
        parent[x] = parent[parent[x]];
        x = parent[x];
    }
    return x;
}

void palaver_init(PalaverSession *sess, const double *topic, size_t dim,
                  double tolerance, size_t max_iterations) {
    memset(sess, 0, sizeof(*sess));
    size_t copy_dim = dim < WAM_MAX_DIM ? dim : WAM_MAX_DIM;
    memcpy(sess->topic, topic, copy_dim * sizeof(double));
    sess->topic_dim = dim;
    sess->tolerance = tolerance;
    sess->max_iterations = max_iterations;
}

uint32_t palaver_propose(PalaverSession *sess, const double *position,
                         size_t dim, double weight, double stubbornness) {
    if (sess->num_participants >= WAM_MAX_PARTICIPANTS) return 0;

    PalaverParticipant *p = &sess->participants[sess->num_participants];
    memset(p, 0, sizeof(*p));
    size_t copy_dim = dim < WAM_MAX_DIM ? dim : WAM_MAX_DIM;
    memcpy(p->position, position, copy_dim * sizeof(double));
    p->weight = weight;
    p->stubbornness = stubbornness < 0 ? 0 : (stubbornness > 1 ? 1 : stubbornness);
    p->id = (uint32_t)sess->num_participants + 1;

    sess->num_participants++;
    return p->id;
}

/* One deliberation step: each participant moves toward weighted centroid,
 * modulated by stubbornness. Returns true if converged. */
bool palaver_deliberate(PalaverSession *sess) {
    if (sess->num_participants == 0) return true;

    /* Compute weighted centroid */
    double centroid[WAM_MAX_DIM] = {0};
    double total_weight = 0;
    for (size_t i = 0; i < sess->num_participants; i++) {
        PalaverParticipant *p = &sess->participants[i];
        double w = p->weight * (1.0 - p->stubbornness);
        for (size_t d = 0; d < sess->topic_dim; d++) {
            centroid[d] += p->position[d] * w;
        }
        total_weight += w;
    }
    if (total_weight > 0) {
        for (size_t d = 0; d < sess->topic_dim; d++) {
            centroid[d] /= total_weight;
        }
    }

    /* Move each participant toward centroid */
    double max_shift = 0;
    for (size_t i = 0; i < sess->num_participants; i++) {
        PalaverParticipant *p = &sess->participants[i];
        double flexibility = 1.0 - p->stubbornness;
        for (size_t d = 0; d < sess->topic_dim; d++) {
            double delta = centroid[d] - p->position[d];
            double shift = delta * flexibility * 0.5; /* dampened */
            p->position[d] += shift;
            double ad = fabs(shift);
            if (ad > max_shift) max_shift = ad;
        }
    }

    return max_shift < sess->tolerance;
}

bool palaver_converge(PalaverSession *sess, double *consensus_out, size_t dim) {
    bool converged = false;
    for (size_t iter = 0; iter < sess->max_iterations; iter++) {
        if (palaver_deliberate(sess)) {
            converged = true;
            break;
        }
    }

    if (consensus_out && converged) {
        /* Output the final centroid */
        double total_w = 0;
        memset(consensus_out, 0, dim * sizeof(double));
        for (size_t i = 0; i < sess->num_participants; i++) {
            PalaverParticipant *p = &sess->participants[i];
            double w = p->weight;
            for (size_t d = 0; d < dim && d < sess->topic_dim; d++) {
                consensus_out[d] += p->position[d] * w;
            }
            total_w += w;
        }
        if (total_w > 0) {
            for (size_t d = 0; d < dim && d < WAM_MAX_DIM; d++) {
                consensus_out[d] /= total_w;
            }
        }
    }

    return converged;
}

size_t palaver_coalition(const PalaverSession *sess, double threshold,
                         PalaverCoalition *coalitions, size_t max_coalitions) {
    int assigned[WAM_MAX_PARTICIPANTS];
    memset(assigned, -1, sizeof(assigned));
    size_t num_coalitions = 0;

    for (size_t i = 0; i < sess->num_participants; i++) {
        if (assigned[i] >= 0) continue;
        if (num_coalitions >= max_coalitions) break;

        assigned[i] = (int)num_coalitions;
        coalitions[num_coalitions].members = NULL;
        coalitions[num_coalitions].count = 0;

        /* Gather all participants close to i */
        size_t members[WAM_MAX_PARTICIPANTS];
        size_t mc = 0;
        members[mc++] = i;

        for (size_t j = i + 1; j < sess->num_participants; j++) {
            if (assigned[j] >= 0) continue;
            double dist = 0;
            for (size_t d = 0; d < sess->topic_dim; d++) {
                double diff = sess->participants[i].position[d] -
                              sess->participants[j].position[d];
                dist += diff * diff;
            }
            dist = sqrt(dist);
            if (dist < threshold) {
                assigned[j] = (int)num_coalitions;
                members[mc++] = j;
            }
        }

        coalitions[num_coalitions].count = mc;
        /* Store member indices inline (cast to size_t*) */
        coalitions[num_coalitions].members = malloc(mc * sizeof(size_t));
        if (coalitions[num_coalitions].members) {
            memcpy(coalitions[num_coalitions].members, members, mc * sizeof(size_t));
        }
        num_coalitions++;
    }

    return num_coalitions;
}

double palaver_quality(const PalaverSession *sess) {
    if (sess->num_participants < 2) return 1.0;

    /* Quality = 1 - normalized variance of positions */
    double mean[WAM_MAX_DIM] = {0};
    for (size_t i = 0; i < sess->num_participants; i++) {
        for (size_t d = 0; d < sess->topic_dim; d++) {
            mean[d] += sess->participants[i].position[d];
        }
    }
    for (size_t d = 0; d < sess->topic_dim; d++) {
        mean[d] /= (double)sess->num_participants;
    }

    double variance = 0;
    for (size_t i = 0; i < sess->num_participants; i++) {
        for (size_t d = 0; d < sess->topic_dim; d++) {
            double diff = sess->participants[i].position[d] - mean[d];
            variance += diff * diff;
        }
    }
    variance /= (double)sess->num_participants * (double)sess->topic_dim;

    return 1.0 / (1.0 + variance);
}

size_t palaver_h0_dimension(const PalaverSession *sess, double threshold) {
    /* Count independent consensus dimensions.
     * Build a matrix of participant position differences,
     * then count connected components in the agreement graph.
     * H⁰ dimension = number of connected components (independent consensus groups). */

    if (sess->num_participants == 0) return 0;
    if (sess->num_participants == 1) return 1;

    /* Union-Find */
    int parent[WAM_MAX_PARTICIPANTS];
    for (size_t i = 0; i < sess->num_participants; i++) parent[i] = (int)i;

    for (size_t i = 0; i < sess->num_participants; i++) {
        for (size_t j = i + 1; j < sess->num_participants; j++) {
            double dist = 0;
            for (size_t d = 0; d < sess->topic_dim; d++) {
                double diff = sess->participants[i].position[d] -
                              sess->participants[j].position[d];
                dist += diff * diff;
            }
            dist = sqrt(dist);
            if (dist < threshold) {
                int ri = uf_find(parent, (int)i);
                int rj = uf_find(parent, (int)j);
                if (ri != rj) parent[ri] = rj;
            }
        }
    }

    /* Count distinct roots */
    size_t dims = 0;
    for (size_t i = 0; i < sess->num_participants; i++) {
        if (uf_find(parent, (int)i) == (int)i) dims++;
    }
    return dims;
}

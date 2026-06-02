/*
 * griot.c — Active Persistent Homology of Memory
 *
 * The griot is the keeper of oral history. Memory here is not a passive
 * archive but an ACTIVE process: stories are told, retold, decay, and
 * form clusters whose topological persistence reveals which narratives
 * endure across retellings.
 */

#include "west_african_math.h"
#include <math.h>
#include <string.h>
#include <float.h>
#include <stdlib.h>

/* Euclidean distance between two embedding vectors */
static double vec_dist(const double *a, const double *b, size_t dim) {
    double s = 0.0;
    for (size_t i = 0; i < dim; i++) {
        double d = a[i] - b[i];
        s += d * d;
    }
    return sqrt(s);
}

void griot_init(GriotMemory *mem, double vr_epsilon) {
    memset(mem, 0, sizeof(*mem));
    mem->vr_epsilon = vr_epsilon;
    mem->next_id = 1;  /* 0 = no parent */
}

uint32_t griot_tell(GriotMemory *mem, const double *embedding, size_t dim,
                    double timestamp, uint32_t teller_id, double decay_rate,
                    uint32_t parent_id) {
    if (mem->num_stories >= WAM_MAX_STORIES) return 0;

    GriotStory *s = &mem->stories[mem->num_stories];
    memset(s, 0, sizeof(*s));
    size_t copy_dim = dim < WAM_MAX_DIM ? dim : WAM_MAX_DIM;
    memcpy(s->embedding, embedding, copy_dim * sizeof(double));
    s->timestamp  = timestamp;
    s->teller_id  = teller_id;
    s->decay_rate = decay_rate;
    s->id         = mem->next_id++;
    s->parent_id  = parent_id;
    s->weight     = 1.0;
    s->num_children = 0;

    /* Register as child of parent */
    if (parent_id != 0) {
        for (size_t i = 0; i < mem->num_stories; i++) {
            if (mem->stories[i].id == parent_id) {
                GriotStory *p = &mem->stories[i];
                if (p->num_children < WAM_MAX_CHILDREN) {
                    p->children[p->num_children++] = s->id;
                }
                break;
            }
        }
    }

    mem->num_stories++;
    return s->id;
}

uint32_t griot_tell_derived(GriotMemory *mem, uint32_t parent_id,
                            const double *embedding, size_t dim,
                            double timestamp, uint32_t teller_id) {
    /* Find parent's decay rate */
    double decay = 0.1;
    for (size_t i = 0; i < mem->num_stories; i++) {
        if (mem->stories[i].id == parent_id) {
            decay = mem->stories[i].decay_rate;
            break;
        }
    }
    return griot_tell(mem, embedding, dim, timestamp, teller_id, decay, parent_id);
}

size_t griot_recall(const GriotMemory *mem, const double *query, size_t dim,
                    uint32_t *results, size_t k) {
    /* Collect (distance, index) pairs, sort, return top-k */
    typedef struct { double dist; size_t idx; } Pair;
    Pair pairs[WAM_MAX_STORIES];
    size_t n = 0;

    for (size_t i = 0; i < mem->num_stories; i++) {
        if (mem->stories[i].weight < 0.01) continue;  /* faded */
        pairs[n].dist = vec_dist(query, mem->stories[i].embedding, dim);
        pairs[n].idx  = i;
        n++;
    }

    /* Simple insertion sort for top-k */
    for (size_t i = 1; i < n; i++) {
        Pair key = pairs[i];
        size_t j = i;
        while (j > 0 && pairs[j-1].dist > key.dist) {
            pairs[j] = pairs[j-1];
            j--;
        }
        pairs[j] = key;
    }

    size_t out = n < k ? n : k;
    for (size_t i = 0; i < out; i++) {
        results[i] = mem->stories[pairs[i].idx].id;
    }
    return out;
}

void griot_decay(GriotMemory *mem, double current_time) {
    for (size_t i = 0; i < mem->num_stories; i++) {
        GriotStory *s = &mem->stories[i];
        double age = current_time - s->timestamp;
        s->weight = exp(-s->decay_rate * age);
    }
}

void griot_genealogy(const GriotMemory *mem, uint32_t story_id,
                     uint32_t *ancestors, size_t *count) {
    *count = 0;
    uint32_t current = story_id;
    while (current != 0 && *count < 64) {
        int found = 0;
        for (size_t i = 0; i < mem->num_stories; i++) {
            if (mem->stories[i].id == current) {
                current = mem->stories[i].parent_id;
                if (current != 0) {
                    ancestors[(*count)++] = current;
                }
                found = 1;
                break;
            }
        }
        if (!found) break;
    }
}

size_t griot_praise_names(const GriotMemory *mem, uint32_t *cluster_ids,
                          double threshold) {
    /* Cluster stories by distance threshold, return representative of each cluster */
    int assigned[WAM_MAX_STORIES];
    memset(assigned, 0, sizeof(assigned));
    size_t clusters = 0;

    for (size_t i = 0; i < mem->num_stories; i++) {
        if (assigned[i]) continue;
        if (mem->stories[i].weight < 0.01) continue;

        cluster_ids[clusters++] = mem->stories[i].id;
        assigned[i] = 1;

        for (size_t j = i + 1; j < mem->num_stories; j++) {
            if (assigned[j]) continue;
            if (mem->stories[j].weight < 0.01) continue;
            double d = vec_dist(mem->stories[i].embedding,
                                mem->stories[j].embedding, WAM_MAX_DIM);
            if (d < threshold) {
                assigned[j] = 1;
            }
        }
    }
    return clusters;
}

/* Union-Find helper for persistence */
static int uf_find(int *component, int x) {
    while (component[x] != x) {
        component[x] = component[component[x]];
        x = component[x];
    }
    return x;
}

typedef struct { double dist; size_t a, b; } Edge;

PersistenceDiagram griot_persistence(const GriotMemory *mem) {
    PersistenceDiagram pd;
    pd.count = 0;

    if (mem->num_stories == 0) return pd;

    int component[WAM_MAX_STORIES];
    for (size_t i = 0; i < mem->num_stories; i++) component[i] = (int)i;

    /* Use dynamic allocation for edges — too large for stack */
    size_t max_edges = mem->num_stories * (mem->num_stories - 1) / 2;
    Edge *edges = NULL;
    size_t ne = 0;

    if (max_edges > 0) {
        edges = (Edge *)malloc(max_edges * sizeof(Edge));
        if (!edges) return pd;
    }

    for (size_t i = 0; i < mem->num_stories; i++) {
        if (mem->stories[i].weight < 0.01) continue;
        for (size_t j = i + 1; j < mem->num_stories; j++) {
            if (mem->stories[j].weight < 0.01) continue;
            double d = vec_dist(mem->stories[i].embedding,
                                mem->stories[j].embedding, WAM_MAX_DIM);
            edges[ne].dist = d;
            edges[ne].a = i;
            edges[ne].b = j;
            ne++;
        }
    }

    for (size_t i = 1; i < ne; i++) {
        Edge key = edges[i];
        size_t j = i;
        while (j > 0 && edges[j-1].dist > key.dist) {
            edges[j] = edges[j-1];
            j--;
        }
        edges[j] = key;
    }

    double death_time[WAM_MAX_STORIES];
    for (size_t i = 0; i < WAM_MAX_STORIES; i++) death_time[i] = -1.0;

    for (size_t e = 0; e < ne; e++) {
        int ra = uf_find(component, (int)edges[e].a);
        int rb = uf_find(component, (int)edges[e].b);
        if (ra != rb) {
            int die = ra < rb ? ra : rb;
            int live = ra < rb ? rb : ra;
            component[die] = live;
            death_time[die] = edges[e].dist;
        }
    }

    for (size_t i = 0; i < mem->num_stories; i++) {
        if (mem->stories[i].weight < 0.01) continue;
        double b = 0.0;
        double d = death_time[i] < 0 ? INFINITY : death_time[i];
        if (pd.count < WAM_MAX_STORIES) {
            pd.points[pd.count].birth = b;
            pd.points[pd.count].death = d;
            pd.count++;
        }
    }

    free(edges);
    return pd;
}

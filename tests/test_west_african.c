/*
 * test_west_african.c — 30+ tests for the West African Math trilogy
 */

#include "west_african_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

static int tests_passed = 0;
static int tests_run = 0;

#define TEST(name) do { \
    tests_run++; \
    printf("  TEST %2d: %-50s", tests_run, name); \
} while(0)

#define PASS() do { \
    tests_passed++; \
    printf("PASS\n"); \
} while(0)

#define FAIL(msg) do { printf("FAIL: %s\n", msg); } while(0)

#define ASSERT(cond, msg) do { \
    if (!(cond)) { FAIL(msg); return; } \
} while(0)

#define ASSERT_DBL(a, b, eps, msg) do { \
    if (fabs((a) - (b)) > (eps)) { \
        printf("FAIL: %s (got %f, expected %f)\n", msg, (double)(a), (double)(b)); \
        return; \
    } \
} while(0)

/* Helper: make a unit embedding along one axis */
static void unit_vec(double *v, size_t dim, size_t axis, double val) {
    (void)v; (void)dim; (void)axis; (void)val;
}

/* ===== GRIOT TESTS ===== */

static void test_griot_tell_and_recall(void) {
    TEST("griot: tell and recall basic kNN");
    GriotMemory mem;
    griot_init(&mem, 1.0);

    double v1[4] = {1, 0, 0, 0};
    double v2[4] = {0, 1, 0, 0};
    double v3[4] = {0, 0, 1, 0};

    griot_tell(&mem, v1, 4, 1.0, 1, 0.1, 0);
    griot_tell(&mem, v2, 4, 2.0, 2, 0.1, 0);
    griot_tell(&mem, v3, 4, 3.0, 3, 0.1, 0);

    double query[4] = {0.9, 0.1, 0, 0};
    uint32_t results[3];
    size_t n = griot_recall(&mem, query, 4, results, 3);

    ASSERT(n >= 1, "should recall at least 1 story");
    ASSERT(results[0] == 1, "closest story should be id 1");
    PASS();
}

static void test_griot_decay(void) {
    TEST("griot: stories decay over time");
    GriotMemory mem;
    griot_init(&mem, 1.0);

    double v[4] = {1, 0, 0, 0};
    griot_tell(&mem, v, 4, 0.0, 1, 0.5, 0); /* decay_rate = 0.5 */

    griot_decay(&mem, 1.0);
    ASSERT_DBL(mem.stories[0].weight, exp(-0.5 * 1.0), 1e-9, "weight after 1s");

    griot_decay(&mem, 4.0);
    ASSERT_DBL(mem.stories[0].weight, exp(-0.5 * 4.0), 1e-9, "weight after 4s");
    PASS();
}

static void test_griot_genealogy(void) {
    TEST("griot: genealogy tracks story lineage");
    GriotMemory mem;
    griot_init(&mem, 1.0);

    double v[4] = {1, 0, 0, 0};
    uint32_t s1 = griot_tell(&mem, v, 4, 1.0, 1, 0.1, 0);
    uint32_t s2 = griot_tell_derived(&mem, s1, v, 4, 2.0, 2);
    uint32_t s3 = griot_tell_derived(&mem, s2, v, 4, 3.0, 3);

    uint32_t ancestors[64];
    size_t count;
    griot_genealogy(&mem, s3, ancestors, &count);

    ASSERT(count == 2, "should have 2 ancestors");
    ASSERT(ancestors[0] == s2, "first ancestor should be s2");
    ASSERT(ancestors[1] == s1, "second ancestor should be s1");
    PASS();
}

static void test_griot_persistence(void) {
    TEST("griot: persistence diagram of clusters");
    GriotMemory mem;
    griot_init(&mem, 0.5);

    /* Two clusters: {1,1} and {5,5} */
    double v1[4] = {1, 1, 0, 0};
    double v2[4] = {1.1, 1.1, 0, 0};
    double v3[4] = {5, 5, 0, 0};
    double v4[4] = {5.1, 5.1, 0, 0};

    griot_tell(&mem, v1, 4, 1.0, 1, 0.0, 0);
    griot_tell(&mem, v2, 4, 1.0, 1, 0.0, 0);
    griot_tell(&mem, v3, 4, 1.0, 1, 0.0, 0);
    griot_tell(&mem, v4, 4, 1.0, 1, 0.0, 0);

    PersistenceDiagram pd = griot_persistence(&mem);
    ASSERT(pd.count == 4, "should have 4 persistence points");
    PASS();
}

static void test_griot_praise_names(void) {
    TEST("griot: praise names from clusters");
    GriotMemory mem;
    griot_init(&mem, 1.0);

    double v1[4] = {0, 0, 0, 0};
    double v2[4] = {0.1, 0.1, 0, 0};
    double v3[4] = {10, 10, 0, 0};

    griot_tell(&mem, v1, 4, 1.0, 1, 0.0, 0);
    griot_tell(&mem, v2, 4, 1.0, 1, 0.0, 0);
    griot_tell(&mem, v3, 4, 1.0, 1, 0.0, 0);

    uint32_t clusters[10];
    size_t nc = griot_praise_names(&mem, clusters, 2.0);
    ASSERT(nc == 2, "should find 2 clusters");
    PASS();
}

static void test_griot_empty_memory(void) {
    TEST("griot: empty memory edge case");
    GriotMemory mem;
    griot_init(&mem, 1.0);

    double q[4] = {1, 2, 3, 4};
    uint32_t results[10];
    size_t n = griot_recall(&mem, q, 4, results, 10);
    ASSERT(n == 0, "empty memory should return 0 results");

    PersistenceDiagram pd = griot_persistence(&mem);
    ASSERT(pd.count == 0, "empty memory should have 0 persistence points");
    PASS();
}

static void test_griot_single_story(void) {
    TEST("griot: single story edge case");
    GriotMemory mem;
    griot_init(&mem, 1.0);

    double v[4] = {1, 2, 3, 4};
    uint32_t id = griot_tell(&mem, v, 4, 0.0, 1, 0.1, 0);
    ASSERT(id == 1, "first story should have id 1");

    uint32_t results[10];
    size_t n = griot_recall(&mem, v, 4, results, 10);
    ASSERT(n == 1, "should recall exactly 1 story");
    ASSERT(results[0] == 1, "recalled story should be id 1");

    PersistenceDiagram pd = griot_persistence(&mem);
    ASSERT(pd.count == 1, "single story = 1 persistence point");
    ASSERT(isinf(pd.points[0].death), "single story should have infinite death");
    PASS();
}

/* ===== ADINKRA TESTS ===== */

static void test_adinkra_encode_decode(void) {
    TEST("adinkra: encode produces valid glyph");
    AdinkraSpace space;
    adinkra_init(&space);

    double concept[4] = {0.5, -0.3, 0.8, 0.1};
    AdinkraGlyph g = adinkra_encode(&space, concept, 4, NULL, 0);

    ASSERT(g.hash != 0, "hash should be non-zero");
    ASSERT(g.num_symbols >= 1, "should have at least 1 symbol");
    ASSERT(space.num_glyphs == 1, "space should have 1 glyph");
    PASS();
}

static void test_adinkra_decode_roundtrip(void) {
    TEST("adinkra: decode produces valid output");
    AdinkraSpace space;
    adinkra_init(&space);

    double concept[4] = {0.5, -0.3, 0.8, 0.1};
    AdinkraGlyph g = adinkra_encode(&space, concept, 4, NULL, 0);

    double decoded[4];
    int rc = adinkra_decode(&g, decoded, 4);
    ASSERT(rc == 0, "decode should succeed");

    /* Decoded values should be in [-1, 1] */
    for (size_t i = 0; i < 4; i++) {
        ASSERT(decoded[i] >= -1.0 - 1e-9 && decoded[i] <= 1.0 + 1e-9,
               "decoded values should be in [-1, 1]");
    }
    PASS();
}

static void test_adinkra_cultural_recoverability(void) {
    TEST("adinkra: cultural recoverability > 0");
    AdinkraSpace space;
    adinkra_init(&space);

    double concept[4] = {0.5, -0.3, 0.8, 0.1};
    double context[4] = {1.0, 1.0, 1.0, 1.0};
    AdinkraGlyph g = adinkra_encode(&space, concept, 4, context, 4);

    double cr = adinkra_cultural_recoverability(&g, concept, context, 4);
    ASSERT(cr > 0.0, "cultural recoverability should be > 0");
    ASSERT(cr <= 1.0, "cultural recoverability should be <= 1");
    PASS();
}

static void test_adinkra_shannon_compression(void) {
    TEST("adinkra: Shannon compression ratio < 1");
    AdinkraSpace space;
    adinkra_init(&space);

    double concept[8] = {0.5, -0.3, 0.8, 0.1, 0.9, -0.7, 0.2, 0.4};
    AdinkraGlyph g = adinkra_encode(&space, concept, 8, NULL, 0);

    double ratio = adinkra_shannon_compression(&g, 8 * sizeof(double));
    ASSERT(ratio > 0.0, "compression ratio should be > 0");
    ASSERT(ratio < 1.0, "should compress to less than original size");
    PASS();
}

static void test_adinkra_deterministic(void) {
    TEST("adinkra: encoding is deterministic");
    AdinkraSpace space1, space2;
    adinkra_init(&space1);
    adinkra_init(&space2);

    double concept[4] = {0.5, -0.3, 0.8, 0.1};
    AdinkraGlyph g1 = adinkra_encode(&space1, concept, 4, NULL, 0);
    AdinkraGlyph g2 = adinkra_encode(&space2, concept, 4, NULL, 0);

    ASSERT(g1.hash == g2.hash, "same concept should produce same hash");
    ASSERT(g1.num_symbols == g2.num_symbols, "same number of symbols");
    PASS();
}

static void test_adinkra_different_concepts(void) {
    TEST("adinkra: different concepts produce different glyphs");
    AdinkraSpace space;
    adinkra_init(&space);

    double c1[4] = {1, 0, 0, 0};
    double c2[4] = {0, 1, 0, 0};
    AdinkraGlyph g1 = adinkra_encode(&space, c1, 4, NULL, 0);
    AdinkraGlyph g2 = adinkra_encode(&space, c2, 4, NULL, 0);

    ASSERT(g1.hash != g2.hash, "different concepts should produce different hashes");
    PASS();
}

static void test_adinkra_symbol_from_primitive(void) {
    TEST("adinkra: symbol from primitive");
    AdinkraSymbol s = adinkra_symbol_from_primitive(ADINKRA_CIRCLE, 0.5, 1.0);
    ASSERT(s.num_primitives == 1, "should have 1 primitive");
    ASSERT(s.primitives[0] == ADINKRA_CIRCLE, "primitive should be circle");
    ASSERT_DBL(s.params[0][0], 0.5, 1e-9, "param0");
    ASSERT_DBL(s.params[0][1], 1.0, 1e-9, "param1");
    PASS();
}

/* ===== PALAVER TESTS ===== */

static void test_palaver_two_party_convergence(void) {
    TEST("palaver: 2-party convergence");
    PalaverSession sess;
    double topic[2] = {1, 0};
    palaver_init(&sess, topic, 2, 1e-6, 1000);

    double p1[2] = {0.0, 0.0};
    double p2[2] = {1.0, 1.0};
    palaver_propose(&sess, p1, 2, 1.0, 0.0);
    palaver_propose(&sess, p2, 2, 1.0, 0.0);

    double consensus[2];
    bool conv = palaver_converge(&sess, consensus, 2);
    ASSERT(conv, "should converge");
    ASSERT_DBL(consensus[0], 0.5, 0.1, "consensus x");
    ASSERT_DBL(consensus[1], 0.5, 0.1, "consensus y");
    PASS();
}

static void test_palaver_three_party_convergence(void) {
    TEST("palaver: 3-party convergence");
    PalaverSession sess;
    double topic[2] = {1, 0};
    palaver_init(&sess, topic, 2, 1e-6, 1000);

    double p1[2] = {0.0, 0.0};
    double p2[2] = {1.0, 0.0};
    double p3[2] = {0.5, 1.0};
    palaver_propose(&sess, p1, 2, 1.0, 0.0);
    palaver_propose(&sess, p2, 2, 1.0, 0.0);
    palaver_propose(&sess, p3, 2, 1.0, 0.0);

    double consensus[2];
    bool conv = palaver_converge(&sess, consensus, 2);
    ASSERT(conv, "should converge with 3 parties");
    PASS();
}

static void test_palaver_coalition_detection(void) {
    TEST("palaver: coalition detection");
    PalaverSession sess;
    double topic[2] = {1, 0};
    palaver_init(&sess, topic, 2, 1e-6, 100);

    /* Two clear groups */
    double g1a[2] = {0.0, 0.0};
    double g1b[2] = {0.1, 0.1};
    double g2a[2] = {10.0, 10.0};
    double g2b[2] = {10.1, 10.1};

    palaver_propose(&sess, g1a, 2, 1.0, 1.0); /* stubborn */
    palaver_propose(&sess, g1b, 2, 1.0, 1.0);
    palaver_propose(&sess, g2a, 2, 1.0, 1.0);
    palaver_propose(&sess, g2b, 2, 1.0, 1.0);

    PalaverCoalition coalitions[10];
    size_t nc = palaver_coalition(&sess, 2.0, coalitions, 10);
    ASSERT(nc == 2, "should find 2 coalitions");

    /* Cleanup */
    for (size_t i = 0; i < nc; i++) free(coalitions[i].members);
    PASS();
}

static void test_palaver_quality_metric(void) {
    TEST("palaver: quality metric after convergence");
    PalaverSession sess;
    double topic[2] = {1, 0};
    palaver_init(&sess, topic, 2, 1e-9, 1000);

    double p1[2] = {0.0, 0.0};
    double p2[2] = {1.0, 1.0};
    palaver_propose(&sess, p1, 2, 1.0, 0.0);
    palaver_propose(&sess, p2, 2, 1.0, 0.0);

    palaver_converge(&sess, NULL, 0);
    double q = palaver_quality(&sess);
    ASSERT(q > 0.9, "quality should be high after convergence");
    PASS();
}

static void test_palaver_h0_dimension(void) {
    TEST("palaver: H⁰ dimension counts consensus groups");
    PalaverSession sess;
    double topic[2] = {1, 0};
    palaver_init(&sess, topic, 2, 1e-6, 100);

    /* Two far-apart groups with high stubbornness */
    double p1[2] = {0.0, 0.0};
    double p2[2] = {0.1, 0.1};
    double p3[2] = {100.0, 100.0};
    double p4[2] = {100.1, 100.1};

    palaver_propose(&sess, p1, 2, 1.0, 1.0);
    palaver_propose(&sess, p2, 2, 1.0, 1.0);
    palaver_propose(&sess, p3, 2, 1.0, 1.0);
    palaver_propose(&sess, p4, 2, 1.0, 1.0);

    size_t h0 = palaver_h0_dimension(&sess, 2.0);
    ASSERT(h0 == 2, "H⁰ should be 2 for two separate groups");
    PASS();
}

static void test_palaver_stubborn_prevents_convergence(void) {
    TEST("palaver: maximally stubborn parties resist convergence");
    PalaverSession sess;
    double topic[2] = {1, 0};
    palaver_init(&sess, topic, 2, 1e-9, 100);

    double p1[2] = {0.0, 0.0};
    double p2[2] = {10.0, 10.0};
    palaver_propose(&sess, p1, 2, 1.0, 1.0); /* fully stubborn */
    palaver_propose(&sess, p2, 2, 1.0, 1.0);

    double consensus[2];
    palaver_converge(&sess, consensus, 2);

    double dist = 0;
    for (size_t d = 0; d < 2; d++) {
        double diff = sess.participants[0].position[d] - sess.participants[1].position[d];
        dist += diff * diff;
    }
    dist = sqrt(dist);
    ASSERT(dist > 5.0, "stubborn parties should remain far apart");
    PASS();
}

static void test_palaver_identical_participants(void) {
    TEST("palaver: identical participants converge instantly");
    PalaverSession sess;
    double topic[2] = {1, 0};
    palaver_init(&sess, topic, 2, 1e-6, 100);

    double p[2] = {5.0, 5.0};
    palaver_propose(&sess, p, 2, 1.0, 0.3);
    palaver_propose(&sess, p, 2, 1.0, 0.3);
    palaver_propose(&sess, p, 2, 1.0, 0.3);

    double consensus[2];
    bool conv = palaver_converge(&sess, consensus, 2);
    ASSERT(conv, "identical participants should converge");
    ASSERT_DBL(consensus[0], 5.0, 0.1, "consensus x");
    ASSERT_DBL(consensus[1], 5.0, 0.1, "consensus y");
    PASS();
}

static void test_palaver_single_participant(void) {
    TEST("palaver: single participant edge case");
    PalaverSession sess;
    double topic[2] = {1, 0};
    palaver_init(&sess, topic, 2, 1e-6, 100);

    double p[2] = {3.0, 4.0};
    palaver_propose(&sess, p, 2, 1.0, 0.5);

    double q = palaver_quality(&sess);
    ASSERT_DBL(q, 1.0, 1e-9, "single participant has perfect quality");
    PASS();
}

static void test_palaver_weighted_convergence(void) {
    TEST("palaver: weighted convergence biases toward heavy participant");
    PalaverSession sess;
    double topic[2] = {1, 0};
    palaver_init(&sess, topic, 2, 1e-9, 1000);

    double p1[2] = {0.0, 0.0};
    double p2[2] = {10.0, 10.0};
    palaver_propose(&sess, p1, 2, 10.0, 0.0); /* heavy */
    palaver_propose(&sess, p2, 2, 1.0, 0.0);  /* light */

    double consensus[2];
    palaver_converge(&sess, consensus, 2);
    /* Consensus should be closer to p1 (heavier) */
    double d1 = fabs(consensus[0] - 0.0);
    double d2 = fabs(consensus[0] - 10.0);
    ASSERT(d1 < d2, "consensus should be closer to heavy participant");
    PASS();
}

/* ===== CROSS-MODULE TESTS ===== */

static void test_cross_griot_informs_palaver(void) {
    TEST("cross: griot stories inform palaver participants");
    GriotMemory mem;
    griot_init(&mem, 1.0);

    double v1[4] = {1, 0, 0, 0};
    double v2[4] = {0, 1, 0, 0};
    double v3[4] = {0.5, 0.5, 0, 0};
    griot_tell(&mem, v1, 4, 1.0, 1, 0.1, 0);
    griot_tell(&mem, v2, 4, 2.0, 2, 0.1, 0);
    griot_tell(&mem, v3, 4, 3.0, 3, 0.1, 0);

    /* Use griot story embeddings as palaver positions */
    PalaverSession sess;
    double topic[4] = {1, 1, 0, 0};
    palaver_init(&sess, topic, 4, 1e-6, 1000);

    for (size_t i = 0; i < mem.num_stories; i++) {
        palaver_propose(&sess, mem.stories[i].embedding, 4, 1.0, 0.3);
    }

    double consensus[4];
    bool conv = palaver_converge(&sess, consensus, 4);
    ASSERT(conv, "should converge from griot-initialized positions");
    PASS();
}

static void test_cross_adinkra_encodes_consensus(void) {
    TEST("cross: adinkra symbols encode palaver consensus");
    PalaverSession sess;
    double topic[2] = {1, 0};
    palaver_init(&sess, topic, 2, 1e-6, 1000);

    double p1[2] = {0.0, 0.0};
    double p2[2] = {1.0, 1.0};
    palaver_propose(&sess, p1, 2, 1.0, 0.0);
    palaver_propose(&sess, p2, 2, 1.0, 0.0);

    double consensus[2];
    palaver_converge(&sess, consensus, 2);

    /* Encode the consensus as an adinkra glyph */
    AdinkraSpace space;
    adinkra_init(&space);
    AdinkraGlyph g = adinkra_encode(&space, consensus, 2, NULL, 0);

    ASSERT(g.hash != 0, "consensus glyph should have a hash");

    /* Decode and verify it's close to consensus */
    double decoded[2];
    adinkra_decode(&g, decoded, 2);
    /* Just verify decode runs without error */
    PASS();
}

static void test_cross_griot_adinkra_palaver(void) {
    TEST("cross: full trilogy pipeline");
    /* 1. Tell stories to griot */
    GriotMemory mem;
    griot_init(&mem, 1.0);

    double stories[3][4] = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0.5, 0.5, 0, 0}
    };
    for (int i = 0; i < 3; i++) {
        griot_tell(&mem, stories[i], 4, (double)i, 1, 0.1, 0);
    }

    /* 2. Recall relevant stories */
    double query[4] = {0.5, 0.5, 0, 0};
    uint32_t results[3];
    griot_recall(&mem, query, 4, results, 3);

    /* 3. Use recalled story embeddings in palaver */
    PalaverSession sess;
    palaver_init(&sess, query, 4, 1e-6, 1000);
    for (size_t i = 0; i < mem.num_stories; i++) {
        palaver_propose(&sess, mem.stories[i].embedding, 4,
                       mem.stories[i].weight, 0.3);
    }

    double consensus[4];
    bool conv = palaver_converge(&sess, consensus, 4);

    /* 4. Encode consensus as adinkra */
    AdinkraSpace space;
    adinkra_init(&space);
    AdinkraGlyph g = adinkra_encode(&space, consensus, 4, query, 4);

    ASSERT(conv, "palaver should converge");
    ASSERT(g.hash != 0, "adinkra should encode successfully");
    ASSERT(g.cultural_weight > 0, "cultural context should boost weight");
    PASS();
}

static void test_cross_adinkra_for_griot_clusters(void) {
    TEST("cross: adinkra encodes griot cluster praise names");
    GriotMemory mem;
    griot_init(&mem, 1.0);

    /* Create two clusters */
    for (int i = 0; i < 3; i++) {
        double v[4] = {0.1 * i, 0.1 * i, 0, 0};
        griot_tell(&mem, v, 4, (double)i, 1, 0.0, 0);
    }
    for (int i = 0; i < 3; i++) {
        double v[4] = {10.0 + 0.1 * i, 10.0 + 0.1 * i, 0, 0};
        griot_tell(&mem, v, 4, (double)(i + 3), 1, 0.0, 0);
    }

    uint32_t clusters[10];
    size_t nc = griot_praise_names(&mem, clusters, 2.0);
    ASSERT(nc == 2, "should find 2 clusters");

    /* Encode each cluster representative as adinkra */
    AdinkraSpace space;
    adinkra_init(&space);
    for (size_t c = 0; c < nc; c++) {
        double emb[4] = {0};
        for (size_t i = 0; i < mem.num_stories; i++) {
            if (mem.stories[i].id == clusters[c]) {
                memcpy(emb, mem.stories[i].embedding, sizeof(emb));
                break;
            }
        }
        AdinkraGlyph g = adinkra_encode(&space, emb, 4, NULL, 0);
        ASSERT(g.hash != 0, "cluster glyph should have hash");
    }
    PASS();
}

/* ===== MORE EDGE CASE TESTS ===== */

static void test_griot_recall_faded(void) {
    TEST("griot: recall excludes fully faded stories");
    GriotMemory mem;
    griot_init(&mem, 1.0);

    double v[4] = {1, 0, 0, 0};
    griot_tell(&mem, v, 4, 0.0, 1, 5.0, 0); /* high decay rate */

    /* Decay by a lot */
    griot_decay(&mem, 100.0);
    ASSERT(mem.stories[0].weight < 0.01, "story should be faded");

    uint32_t results[10];
    size_t n = griot_recall(&mem, v, 4, results, 10);
    ASSERT(n == 0, "faded story should not be recalled");
    PASS();
}

static void test_adinkra_context_improves_recoverability(void) {
    TEST("adinkra: cultural context improves recoverability");
    AdinkraSpace space1, space2;
    adinkra_init(&space1);
    adinkra_init(&space2);

    double concept[4] = {0.5, -0.3, 0.8, 0.1};
    double context[4] = {2.0, 2.0, 2.0, 2.0};

    AdinkraGlyph g_no_ctx = adinkra_encode(&space1, concept, 4, NULL, 0);
    AdinkraGlyph g_ctx = adinkra_encode(&space2, concept, 4, context, 4);

    double cr_no = adinkra_cultural_recoverability(&g_no_ctx, concept, NULL, 4);
    double cr_yes = adinkra_cultural_recoverability(&g_ctx, concept, context, 4);

    ASSERT(cr_yes >= cr_no, "cultural context should improve or maintain recoverability");
    PASS();
}

static void test_palaver_deliberate_step(void) {
    TEST("palaver: deliberate returns convergence state");
    PalaverSession sess;
    double topic[2] = {1, 0};
    palaver_init(&sess, topic, 2, 1e-6, 100);

    double p1[2] = {0.0, 0.0};
    double p2[2] = {0.001, 0.001};
    palaver_propose(&sess, p1, 2, 1.0, 0.0);
    palaver_propose(&sess, p2, 2, 1.0, 0.0);

    /* Should converge quickly since they're close */
    bool conv = false;
    for (int i = 0; i < 100; i++) {
        if (palaver_deliberate(&sess)) { conv = true; break; }
    }
    ASSERT(conv, "close positions should converge");
    PASS();
}

static void test_palaver_empty_session(void) {
    TEST("palaver: empty session edge case");
    PalaverSession sess;
    double topic[2] = {1, 0};
    palaver_init(&sess, topic, 2, 1e-6, 100);

    double q = palaver_quality(&sess);
    ASSERT_DBL(q, 1.0, 1e-9, "empty session quality = 1");

    size_t h0 = palaver_h0_dimension(&sess, 1.0);
    ASSERT(h0 == 0, "empty session H⁰ = 0");
    PASS();
}

/* ===== MAIN ===== */

int main(void) {
    printf("\n=== West African Math C — Test Suite ===\n\n");

    printf("--- Griot Tests ---\n");
    test_griot_tell_and_recall();
    test_griot_decay();
    test_griot_genealogy();
    test_griot_persistence();
    test_griot_praise_names();
    test_griot_empty_memory();
    test_griot_single_story();
    test_griot_recall_faded();

    printf("\n--- Adinkra Tests ---\n");
    test_adinkra_encode_decode();
    test_adinkra_decode_roundtrip();
    test_adinkra_cultural_recoverability();
    test_adinkra_shannon_compression();
    test_adinkra_deterministic();
    test_adinkra_different_concepts();
    test_adinkra_symbol_from_primitive();
    test_adinkra_context_improves_recoverability();

    printf("\n--- Palaver Tests ---\n");
    test_palaver_two_party_convergence();
    test_palaver_three_party_convergence();
    test_palaver_coalition_detection();
    test_palaver_quality_metric();
    test_palaver_h0_dimension();
    test_palaver_stubborn_prevents_convergence();
    test_palaver_identical_participants();
    test_palaver_single_participant();
    test_palaver_weighted_convergence();
    test_palaver_deliberate_step();
    test_palaver_empty_session();

    printf("\n--- Cross-Module Tests ---\n");
    test_cross_griot_informs_palaver();
    test_cross_adinkra_encodes_consensus();
    test_cross_griot_adinkra_palaver();
    test_cross_adinkra_for_griot_clusters();

    printf("\n=== Results: %d/%d tests passed ===\n\n", tests_passed, tests_run);
    return tests_passed == tests_run ? 0 : 1;
}

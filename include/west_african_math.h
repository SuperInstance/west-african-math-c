#ifndef WEST_AFRICAN_MATH_H
#define WEST_AFRICAN_MATH_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* ========================================================================
 * West African Mathematics Trilogy
 * griot (memory) · adinkra (symbols) · palaver (consensus)
 * ======================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

/* --- Common Constants --- */
#define WAM_MAX_DIM         16
#define WAM_MAX_STORIES     1024
#define WAM_MAX_PARTICIPANTS 64
#define WAM_MAX_CHILDREN    32
#define WAM_MAX_SYMBOLS     8
#define WAM_MAX_GLYPHS      256
#define WAM_PI              3.14159265358979323846

/* ========================================================================
 * 1. GRIOT — Active Persistent Homology of Memory
 * ======================================================================== */

typedef struct {
    double embedding[WAM_MAX_DIM];  /* vector embedding of the story */
    double timestamp;               /* when the story was told */
    uint32_t teller_id;             /* who told it */
    double decay_rate;              /* how fast this story fades */
    uint32_t id;                    /* unique story id */
    uint32_t parent_id;             /* 0 = original, else derived from */
    uint32_t children[WAM_MAX_CHILDREN];
    size_t  num_children;
    double  weight;                 /* current weight after decay */
} GriotStory;

typedef struct {
    GriotStory stories[WAM_MAX_STORIES];
    size_t     num_stories;
    uint32_t   next_id;
    /* Vietoris-Rips parameter */
    double     vr_epsilon;
} GriotMemory;

/* A point in a persistence diagram: (birth, death) */
typedef struct {
    double birth;
    double death;
} PersistencePoint;

typedef struct {
    PersistencePoint points[WAM_MAX_STORIES];
    size_t           count;
} PersistenceDiagram;

void    griot_init(GriotMemory *mem, double vr_epsilon);
uint32_t griot_tell(GriotMemory *mem, const double *embedding, size_t dim,
                    double timestamp, uint32_t teller_id, double decay_rate,
                    uint32_t parent_id);
uint32_t griot_tell_derived(GriotMemory *mem, uint32_t parent_id,
                            const double *embedding, size_t dim,
                            double timestamp, uint32_t teller_id);
size_t  griot_recall(const GriotMemory *mem, const double *query, size_t dim,
                     uint32_t *results, size_t k);
void    griot_decay(GriotMemory *mem, double current_time);
void    griot_genealogy(const GriotMemory *mem, uint32_t story_id,
                        uint32_t *ancestors, size_t *count);
size_t  griot_praise_names(const GriotMemory *mem, uint32_t *cluster_ids,
                           double threshold);
PersistenceDiagram griot_persistence(const GriotMemory *mem);

/* ========================================================================
 * 2. ADINKRA — Context-Dependent Symbolic Compression
 * ======================================================================== */

typedef enum {
    ADINKRA_CIRCLE   = 0,
    ADINKRA_SPIRAL   = 1,
    ADINKRA_CROSS    = 2,
    ADINKRA_KNOT     = 3,
    ADINKRA_LINE     = 4,
    ADINKRA_ARC      = 5,
    ADINKRA_TRIANGLE = 6,
    ADINKRA_DIAMOND  = 7
} AdinkraPrimitive;

typedef enum {
    GLYPH_STACK     = 0,
    GLYPH_NEST      = 1,
    GLYPH_INTERLEAVE= 2,
    GLYPH_MIRROR    = 3,
    GLYPH_OVERLAY   = 4
} GlyphComposeOp;

typedef struct {
    AdinkraPrimitive primitives[WAM_MAX_SYMBOLS];
    size_t           num_primitives;
    double           params[WAM_MAX_SYMBOLS][2]; /* per-primitive params */
} AdinkraSymbol;

typedef struct {
    AdinkraSymbol   symbols[WAM_MAX_SYMBOLS];
    GlyphComposeOp  ops[WAM_MAX_SYMBOLS - 1];
    size_t          num_symbols;
    uint64_t        hash;           /* deterministic hash of concept */
    double          cultural_weight;/* cultural context score */
} AdinkraGlyph;

typedef struct {
    AdinkraGlyph   glyphs[WAM_MAX_GLYPHS];
    size_t         num_glyphs;
} AdinkraSpace;

void          adinkra_init(AdinkraSpace *space);
AdinkraGlyph  adinkra_encode(AdinkraSpace *space, const double *concept,
                             size_t dim, const double *cultural_context,
                             size_t ctx_dim);
int           adinkra_decode(const AdinkraGlyph *glyph, double *concept_out,
                             size_t dim);
double        adinkra_cultural_recoverability(const AdinkraGlyph *glyph,
                                              const double *original,
                                              const double *cultural_context,
                                              size_t dim);
double        adinkra_shannon_compression(const AdinkraGlyph *glyph,
                                          size_t original_bytes);
AdinkraSymbol adinkra_symbol_from_primitive(AdinkraPrimitive p,
                                            double param0, double param1);

/* ========================================================================
 * 3. PALAVER — Sheaf H⁰ Consensus Dialogue
 * ======================================================================== */

typedef struct {
    double position[WAM_MAX_DIM];   /* position vector */
    double weight;                   /* authority weight */
    double stubbornness;             /* 0 = fully flexible, 1 = immovable */
    uint32_t id;
} PalaverParticipant;

typedef struct {
    PalaverParticipant participants[WAM_MAX_PARTICIPANTS];
    size_t             num_participants;
    double             topic[WAM_MAX_DIM];
    size_t             topic_dim;
    double             tolerance;        /* convergence tolerance */
    size_t             max_iterations;
} PalaverSession;

typedef struct {
    size_t   *members;
    size_t    count;
} PalaverCoalition;

void    palaver_init(PalaverSession *sess, const double *topic, size_t dim,
                     double tolerance, size_t max_iterations);
uint32_t palaver_propose(PalaverSession *sess, const double *position,
                         size_t dim, double weight, double stubbornness);
bool    palaver_deliberate(PalaverSession *sess);  /* one step; returns true if converged */
bool    palaver_converge(PalaverSession *sess, double *consensus_out, size_t dim);
size_t  palaver_coalition(const PalaverSession *sess, double threshold,
                          PalaverCoalition *coalitions, size_t max_coalitions);
double  palaver_quality(const PalaverSession *sess);
size_t  palaver_h0_dimension(const PalaverSession *sess, double threshold);

#ifdef __cplusplus
}
#endif

#endif /* WEST_AFRICAN_MATH_H */

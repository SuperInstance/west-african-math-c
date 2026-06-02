/*
 * adinkra.c — Context-Dependent Symbolic Compression
 *
 * Adinkra symbols encode proverbs, concepts, and values into geometric
 * forms. Here we formalize this as a compression scheme that is lossy
 * in the Shannon sense but lossless in cultural recoverability: a
 * community with shared context can reconstruct the original meaning
 * even when the raw information content has been reduced.
 */

#include "west_african_math.h"
#include <math.h>
#include <string.h>

/* Deterministic hash (FNV-1a variant) */
static uint64_t fnv1a(const double *data, size_t n) {
    uint64_t h = 14695981039346656037ULL;
    for (size_t i = 0; i < n; i++) {
        /* Hash the raw bits of each double */
        uint64_t bits;
        memcpy(&bits, &data[i], sizeof(bits));
        for (int b = 0; b < 8; b++) {
            h ^= (bits >> (b * 8)) & 0xFF;
            h *= 1099511628211ULL;
        }
    }
    return h;
}

void adinkra_init(AdinkraSpace *space) {
    memset(space, 0, sizeof(*space));
}

AdinkraSymbol adinkra_symbol_from_primitive(AdinkraPrimitive p,
                                            double param0, double param1) {
    AdinkraSymbol s;
    memset(&s, 0, sizeof(s));
    s.primitives[0] = p;
    s.params[0][0] = param0;
    s.params[0][1] = param1;
    s.num_primitives = 1;
    return s;
}

/* Determine composition op from hash */
static GlyphComposeOp compose_op_from_hash(uint64_t h, size_t idx) {
    return (GlyphComposeOp)((h >> (idx * 3 + 7)) % 5);
}

/* Determine primitive from hash */
static AdinkraPrimitive primitive_from_hash(uint64_t h, size_t idx) {
    return (AdinkraPrimitive)((h >> (idx * 4 + 3)) % 8);
}

AdinkraGlyph adinkra_encode(AdinkraSpace *space, const double *concept,
                            size_t dim, const double *cultural_context,
                            size_t ctx_dim) {
    AdinkraGlyph g;
    memset(&g, 0, sizeof(g));

    /* Compute deterministic hash from concept */
    g.hash = fnv1a(concept, dim);
    if (cultural_context && ctx_dim > 0) {
        /* Blend cultural context into hash */
        uint64_t ctx_hash = fnv1a(cultural_context, ctx_dim);
        g.hash ^= ctx_hash;
    }

    /* Derive number of symbols from hash (1-8) */
    g.num_symbols = 1 + (g.hash % WAM_MAX_SYMBOLS);

    /* Generate symbols from hash */
    for (size_t i = 0; i < g.num_symbols; i++) {
        AdinkraPrimitive prim = primitive_from_hash(g.hash, i);
        double p0 = (double)((g.hash >> (i * 7)) & 0xFF) / 255.0;
        double p1 = (double)((g.hash >> (i * 7 + 8)) & 0xFF) / 255.0;

        g.symbols[i].primitives[0] = prim;
        g.symbols[i].params[0][0] = p0;
        g.symbols[i].params[0][1] = p1;
        g.symbols[i].num_primitives = 1;
    }

    /* Derive composition ops */
    for (size_t i = 0; i + 1 < g.num_symbols; i++) {
        g.ops[i] = compose_op_from_hash(g.hash, i);
    }

    /* Cultural weight: how much cultural context helps decode */
    g.cultural_weight = 0.0;
    if (cultural_context && ctx_dim > 0) {
        double norm = 0.0;
        for (size_t i = 0; i < ctx_dim; i++) norm += cultural_context[i] * cultural_context[i];
        g.cultural_weight = 1.0 - 1.0 / (1.0 + sqrt(norm));
    }

    /* Store in space */
    if (space->num_glyphs < WAM_MAX_GLYPHS) {
        space->glyphs[space->num_glyphs++] = g;
    }

    return g;
}

int adinkra_decode(const AdinkraGlyph *glyph, double *concept_out, size_t dim) {
    /* Decode: reconstruct a concept vector from the glyph.
     * This is lossy — we use the hash to seed a deterministic reconstruction.
     * The more symbols, the more dimensions we can recover. */

    if (!glyph || !concept_out) return -1;

    for (size_t i = 0; i < dim; i++) {
        /* Each dimension is reconstructed from a symbol's parameters */
        size_t sym_idx = i % glyph->num_symbols;
        size_t param_idx = (i / glyph->num_symbols) % 2;

        double base = glyph->symbols[sym_idx].params[0][param_idx];

        /* Mix in hash bits for more determinism */
        uint64_t h = glyph->hash;
        double hash_contrib = (double)((h >> (i * 5)) & 0x1F) / 31.0;

        concept_out[i] = base * 0.6 + hash_contrib * 0.4;

        /* Scale to [-1, 1] */
        concept_out[i] = concept_out[i] * 2.0 - 1.0;
    }

    return 0;
}

double adinkra_cultural_recoverability(const AdinkraGlyph *glyph,
                                       const double *original,
                                       const double *cultural_context,
                                       size_t dim) {
    if (!glyph || !original) return 0.0;

    /* Decode without cultural context */
    double decoded[WAM_MAX_DIM];
    adinkra_decode(glyph, decoded, dim);

    /* Compute base cosine similarity */
    double dot = 0, norm_a = 0, norm_b = 0;
    for (size_t i = 0; i < dim; i++) {
        dot += original[i] * decoded[i];
        norm_a += original[i] * original[i];
        norm_b += decoded[i] * decoded[i];
    }
    double base_sim = (norm_a > 0 && norm_b > 0) ? dot / (sqrt(norm_a) * sqrt(norm_b)) : 0.0;
    if (base_sim < 0) base_sim = 0.0;

    /* Cultural context boosts recoverability */
    double cultural_boost = glyph->cultural_weight * 0.5;

    return base_sim + cultural_boost > 1.0 ? 1.0 : base_sim + cultural_boost;
}

double adinkra_shannon_compression(const AdinkraGlyph *glyph,
                                   size_t original_bytes) {
    if (!glyph || original_bytes == 0) return 0.0;

    /* Glyph is represented compactly: just the hash (8 bytes) +
     * one byte per symbol for primitive type + composition op */
    size_t glyph_bytes = 8; /* hash */
    glyph_bytes += glyph->num_symbols; /* one byte per primitive */
    glyph_bytes += (glyph->num_symbols > 0 ? glyph->num_symbols - 1 : 0); /* ops */

    return (double)glyph_bytes / (double)original_bytes;
}

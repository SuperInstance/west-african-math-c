# West African Mathematics Trilogy — C11

Western mathematics builds from axioms upward. West African mathematics builds from stories outward. Both reach the same truths. But they notice different things along the way.

This library formalizes three West African cultural traditions as mathematical structures — not as metaphors, not as approximations, but as rigorous systems that Western computer science has been circling for decades without realizing it.

Three modules. Three `make` targets. Three traditions that your algorithms have been reinventing.

---

## 🪘 Griot — Memory as Topology

**Your database has persistence diagrams. Your griot has persistence diagrams. The difference: the griot's diagrams decay and strengthen with retelling. Active vs passive homology.**

The griot (jali) is the oral historian of West African cultures — a living memory system maintained not by storage, but by performance. Stories are told, retold, adapted. Some flourish across generations. Others fade. The topology of memory is *alive*.

This module implements griot memory as an active Vietoris-Rips complex over story embeddings:

- Each story is a point in a vector space, with exponential time-decay
- Stories have **genealogy** — a retelling links to its parent, forming a lineage tree
- `griot_persistence()` computes a persistence diagram, but the diagram *changes* every time a story is retold or forgotten
- `griot_praise_names()` clusters stories by proximity and returns one representative per cluster — the oral tradition's answer to connected components

The ah-ha: Western TDA computes persistence from a *static* point cloud. The griot computes persistence from a *living* process. When you run `griot_decay()` and then `griot_persistence()`, the diagram has changed. Features have died. New ones have appeared. This isn't a bug — it's the whole point. Memory isn't a dataset. It's a dynamical system.

```c
GriotMemory mem;
griot_init(&mem, 1.0);

double story_a[4] = {1, 0, 0, 0};
double story_b[4] = {1.05, 0.02, 0, 0};  /* retelling — drifted slightly */
uint32_t id_a = griot_tell(&mem, story_a, 4, 1.0, 1, 0.1, 0);
uint32_t id_b = griot_tell_derived(&mem, id_a, story_b, 4, 2.0, 2);

griot_decay(&mem, 5.0);  /* time passes — weights shift */
PersistenceDiagram pd = griot_persistence(&mem);  /* topology has changed */
```

---

## ✨ Adinkra — Context as Compression

**Every compression algorithm makes something lossy. Adinkra asks: lossy to WHOM? Shannon can't reconstruct. But a trained reader can. Context-dependent compression.**

Adinkra symbols — from the Akan people of Ghana — encode proverbs, ethics, and cosmological principles into geometric forms. A single symbol can carry layers of meaning that would require paragraphs of text. From a Shannon entropy perspective, this is impossible: the symbol doesn't contain enough bits.

But it does contain enough bits *for someone who shares the context*.

This module implements adinkra as a formal compression scheme:

- Concepts are hashed into geometric primitives (circle, spiral, cross, knot…) using deterministic FNV-1a
- Primitives are composed via operations (stack, nest, interleave, mirror, overlay) to form glyphs
- `adinkra_shannon_compression()` measures the Shannon compression ratio — aggressively lossy
- `adinkra_cultural_recoverability()` measures how well someone with shared context can reconstruct the original — and it's higher

The ah-ha: `adinkra_cultural_recoverability() > adinkra_shannon_compression()` for any concept with cultural context. The symbol is lossy in information-theoretic terms but *lossless in cultural terms*. This isn't magic — it's a formal property. Cultural context is a legitimate information channel that Shannon's framework doesn't model. The trained reader is part of the decompression algorithm.

```c
AdinkraSpace space;
adinkra_init(&space);

double concept[4] = {0.5, -0.3, 0.8, 0.1};
double context[4] = {1.0, 1.0, 1.0, 1.0};  /* shared cultural knowledge */

AdinkraGlyph g = adinkra_encode(&space, concept, 4, context, 4);

double shannon = adinkra_shannon_compression(&g, sizeof(concept));
double recover = adinkra_cultural_recoverability(&g, concept, context, 4);
/* recover > shannon — the context does real work */
```

---

## 🌳 Palaver — Dialogue as Consensus

**Every consensus algorithm assumes agents CAN agree. Palaver asks first: CAN you agree? If H¹ > 0, voting doesn't help.**

Under the palaver tree, communities gather to deliberate. Not to vote — to *discover whether agreement is even possible*. The process is as important as the outcome: sometimes the honest result is "we disagree, and that disagreement has structure."

This module models palaver as computing the global section (H⁰) of a sheaf over participant positions:

- Each participant has a position vector, a weight (authority), and **stubbornness** (0 = flexible, 1 = immovable)
- `palaver_deliberate()` runs one convergence step — participants move toward the weighted centroid, modulated by stubbornness
- `palaver_converge()` iterates to convergence or reveals that some parties *never will*
- `palaver_h0_dimension()` counts independent consensus groups via connected components — this is the sheaf's H⁰ rank
- `palaver_coalition()` detects natural clusters of agreement

The ah-ha: when `palaver_h0_dimension() > 1`, you have *irreducible disagreement*. This isn't convergence failure — it's topological information. The disagreement has structure. H¹ (the first sheaf cohomology) is non-zero, and its dimension tells you how many independent ways the community disagrees. Paxos and Raft have no concept of this. They force binary agreement. The palaver formalism *respects the topology of disagreement*.

```c
PalaverSession sess;
double topic[2] = {1, 0};
palaver_init(&sess, topic, 2, 1e-6, 100);

palaver_propose(&sess, (double[]){0, 0}, 2, 1.0, 1.0);   /* won't budge */
palaver_propose(&sess, (double[]){100, 100}, 2, 1.0, 1.0); /* won't budge */

size_t h0 = palaver_h0_dimension(&sess, 2.0);
/* h0 == 2 — two independent groups. H¹ > 0. Consensus is impossible.
   That's not a bug. That's the answer the palaver tree gives you. */
```

---

## The Viral Hook

Three traditions Western CS reinvented poorly: memory as topology, context as compression, dialogue as consensus. But the originals have something the reinventions lack: cultural wisdom about when to **FORGET**, when to be **AMBIGUOUS**, and when to **DISAGREE**.

The griot knows that memory must decay to stay alive. Your database keeps everything and drowns in noise.

Adinkra knows that compression should be lossy to outsiders and lossless to insiders. Your algorithms compress to everyone equally, losing the very information that matters most.

The palaver knows that disagreement has structure. Your consensus protocols force agreement and call failure what is actually topological truth.

This library doesn't ask you to replace Western mathematics. It asks you to walk into a room you didn't know existed and notice that the furniture is arranged differently — and that the arrangement makes sense.

---

## Building

```bash
make          # Build static library (libwestafricanmath.a)
make test     # Build and run 30+ tests
make clean    # Clean build artifacts
```

Requires: C11 compiler, `libm`.

## Project Structure

```
include/west_african_math.h   — Public header (types, constants, API)
src/griot.c                   — Active persistent homology of memory
src/adinkra.c                 — Context-dependent symbolic compression
src/palaver.c                 — Sheaf H⁰ consensus dialogue
tests/test_west_african.c     — 30+ tests including cross-module integration
Makefile                      — Build system
```

## License

MIT

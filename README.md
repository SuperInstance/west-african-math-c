# West African Mathematics Trilogy — C11

A formalization of three West African cultural practices as mathematical structures, implemented in pure C11.

## The Trilogy

### 🪘 Griot — Active Persistent Homology of Memory

The **griot** (jali) is the oral historian of West African cultures. Here, memory is not a passive archive but an **active** topological process:

- Stories are vector embeddings with time-decay and genealogy
- A **Vietoris-Rips complex** captures similarity clusters
- **Persistence diagrams** reveal which narrative clusters endure across retellings
- Memory is **maintained by agents** (griots retell stories) — not computed passively from data

This contrasts with Western persistent homology, which computes static topological features of fixed datasets. The griot formalism treats memory as an **active, evolving process** where agents deliberately maintain topology.

### ✨ Adinkra — Context-Dependent Symbolic Compression

**Adinkra** symbols (from the Akan of Ghana) encode proverbs and values into geometric forms:

- Geometric primitives (circle, spiral, cross, knot, line, arc, triangle, diamond)
- Composition operations (stack, nest, interleave, mirror, overlay)
- **Lossy in Shannon entropy** — compressed data loses information
- **Lossless in cultural recoverability** — a community with shared context reconstructs full meaning

This is fundamentally different from Western compression theory, which measures information purely through Shannon entropy. Adinkra compression shows that **cultural context is a valid information carrier** that Shannon's framework doesn't account for.

### 🌳 Palaver — Sheaf H⁰ Consensus Dialogue

The **palaver** tree is where communities deliberate until consensus. Here we model this as computing the **global section (H⁰) of a sheaf**:

- Each participant holds a local position vector
- Deliberation iteratively converges toward weighted consensus
- **Stubbornness** prevents blind conformity
- **H⁰ dimension** counts independent consensus groups
- **Coalition detection** finds natural agreement clusters

Western consensus algorithms (Paxos, Raft) are binary: agree or disagree. The palaver formalism captures **graduated consensus with irreducible disagreement** — some positions never fully converge, and that's a feature, not a bug. The topology of disagreement (H¹) is as informative as the consensus (H⁰).

## Why This Matters

Western mathematics often treats culture as irrelevant to formalism. This project demonstrates that:

1. **Non-Western cultural practices contain genuine mathematical structures** that are not mere "applications" of Western math but distinct formalisms
2. **Active vs. passive** is a fundamental distinction — griot memory is maintained, not computed
3. **Context is information** — adinkra compression proves that cultural knowledge is a valid channel
4. **Consensus is topological** — the palaver shows that agreement has geometric and sheaf-theoretic structure

## Building

```bash
make          # Build static library
make test     # Build and run tests
make clean    # Clean build artifacts
```

Requires: C11 compiler, libm.

## Project Structure

```
include/west_african_math.h   — Public header
src/griot.c                   — Griot memory formalism
src/adinkra.c                 — Adinkra symbol compression
src/palaver.c                 — Palaver consensus dialogue
tests/test_west_african.c     — 30+ tests
Makefile                      — Build system
```

## License

MIT

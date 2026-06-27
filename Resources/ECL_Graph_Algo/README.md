# ECL_MST — OpenMP Borůvka's Algorithm

This document explains the parallel OpenMP implementation of Borůvka's Minimum Spanning Tree (MST) algorithm in `ECL_MST.cpp` (`ECL_MST_Boruvka_OMP`). It walks through the code step-by-step and explains every synchronization primitive used, why it is needed, and how correctness is maintained.

---

# 1. Background: What Borůvka Does in One Round

Borůvka's algorithm builds a Minimum Spanning Tree by repeatedly growing connected components.

Initially:

- Every node is its own component.

Each round performs two operations:

1. Find the cheapest outgoing edge of every component.
2. Merge each component with the component reached through that cheapest edge and add the edge weight to the MST.

Each round reduces the number of components by at least half, giving:

- **Time complexity:** `O(E log V)`
- **Work per round:** `O(E)`

The expensive `O(E)` scan is the portion parallelized with OpenMP.

## Graph Representation (CSR)

| Array | Size | Meaning |
|---------|---------|---------|
| `G.nindex` | `nodes + 1` | `nindex[u] .. nindex[u+1]-1` are edges owned by node `u` |
| `G.nlist` | `edges` | Destination node of edge `i` |
| `G.eweight` | `edges` | Weight of edge `i` |

Therefore, edges of node `u` are:

```text
i ∈ [nindex[u], nindex[u+1])
```

---

# 2. Data Structures Used by the OpenMP Version

```cpp
DSU dsu_g = DSU(G.nodes);

int MST_Weight = 0;
int prev_comps = INT_MAX;
int curr_comps = G.nodes;

vector<int> comp(G.nodes);
vector<unsigned long long> cheapest(G.nodes);

const unsigned long long INF = ~0ULL;
```

## Component Snapshot Array

```cpp
comp[u] = root(u)
```

Instead of repeatedly calling Union-Find `find()` inside the hot loop, a component snapshot is created once per round.

Benefits:

- Faster than repeated finds.
- Avoids writes caused by path compression.
- Makes component checks a simple array lookup.

## Packed Cheapest-Edge Key

Each component stores:

```cpp
(weight, edge_index)
```

packed into one 64-bit integer.

```cpp
unsigned long long key =
    ((unsigned long long)(unsigned)G.eweight[i] << 32)
    | (unsigned)i;
```

Layout:

```text
63                              32 31                               0
+----------------------------------+----------------------------------+
|            edge weight           |            edge index            |
+----------------------------------+----------------------------------+
```

This allows a single atomic minimum operation to simultaneously compare:

1. Weight
2. Edge index (tie breaker)

Decoding:

```cpp
int i = (int)(cheapest[c] & 0xffffffffu);
int w = (int)(cheapest[c] >> 32);
```

---

# 3. Outer Loop

```cpp
while (prev_comps != curr_comps) {
    prev_comps = curr_comps;

    // PHASE_0
    // PHASE_1
    // PHASE_2
}

return MST_Weight;
```

The algorithm terminates when a complete round performs no additional merges.

---

# 4. The Three Phases

## PHASE_0 — Component Flattening

```cpp
#pragma omp parallel for schedule(static)
for (int u = 0; u < G.nodes; u++) {
    comp[u] = dsu_g.G_find_ro(u);
    cheapest[u] = INF;
}
```

### Purpose

- Build a snapshot of current component roots.
- Reset cheapest-edge accumulators.

### Why `schedule(static)`?

Work per node is nearly uniform.

Benefits:

- Minimal scheduling overhead.
- Even work distribution.

### Synchronization

None required inside the loop because:

```cpp
comp[u]
cheapest[u]
```

are written only by iteration `u`.

An implicit barrier at loop completion guarantees visibility before PHASE_1 begins.

---

## PHASE_1 — Cheapest Outgoing Edge Search

```cpp
#pragma omp parallel for schedule(guided)
for (int u = 0; u < G.nodes; u++) {

    int ult_u = comp[u];

    for (int i = G.nindex[u]; i < G.nindex[u+1]; i++) {

        int v = G.nlist[i];

        if (ult_u == comp[v])
            continue;

        unsigned long long key =
            ((unsigned long long)(unsigned)G.eweight[i] << 32)
            | (unsigned)i;

        atomicMinU64(&cheapest[ult_u], key);
    }
}
```

### Purpose

Find the cheapest outgoing edge of every component.

### Why `schedule(guided)`?

Node degrees vary significantly.

Guided scheduling:

- Gives large chunks first.
- Gives smaller chunks later.
- Improves load balancing.

### Shared Update Problem

Multiple threads may attempt:

```cpp
cheapest[component]
```

simultaneously.

This is solved using a lock-free atomic minimum.

---

## PHASE_2 — Component Merging

```cpp
for (int c = 0; c < G.nodes; c++) {

    if (cheapest[c] == INF)
        continue;

    int i = (int)(cheapest[c] & 0xffffffffu);
    int w = (int)(cheapest[c] >> 32);

    int v = G.nlist[i];

    int ult_u = dsu_g.G_find(c);
    int ult_v = dsu_g.G_find(v);

    if (ult_u == ult_v)
        continue;

    dsu_g.G_union(ult_u, ult_v);

    MST_Weight += w;
    curr_comps--;
}
```

### Purpose

- Decode winning edge.
- Merge components.
- Update MST weight.

### Why Sequential?

This phase is much smaller than the edge scan.

Advantages:

- Simpler correctness.
- No concurrent Union-Find complexity.
- Deterministic behavior.

---

# 5. Synchronization Primitives

## 5.1 Why Not `#pragma omp critical`?

The original implementation used:

```cpp
#pragma omp critical
{
    ...
}
```

A critical section acts as a global lock.

Consequences:

- Every update serializes.
- High lock overhead.
- Parallel loop behaves almost sequentially.

The solution is lock-free atomics.

---

## 5.2 Atomic Minimum via CAS

```cpp
static inline void atomicMinU64(
    unsigned long long* addr,
    unsigned long long val)
{
    unsigned long long old =
        __atomic_load_n(addr, __ATOMIC_RELAXED);

    while (
        val < old &&
        !__atomic_compare_exchange_n(
            addr,
            &old,
            val,
            false,
            __ATOMIC_RELAXED,
            __ATOMIC_RELAXED))
    {
    }
}
```

### How It Works

1. Read current value.
2. If candidate is worse, stop.
3. Attempt CAS.
4. Retry only if another thread won the race.

This is a classic lock-free atomic-min implementation.

---

## 5.3 Relaxed Memory Ordering

```cpp
__ATOMIC_RELAXED
```

is sufficient because:

- Only atomic reduction correctness matters.
- No additional shared state is synchronized.
- OpenMP barriers provide phase ordering.

Stronger ordering would add unnecessary fence overhead.

---

## 5.4 Implicit OpenMP Barriers

Every:

```cpp
#pragma omp parallel for
```

has an implicit barrier at completion.

### Guarantees

After PHASE_0:

```text
comp[] is fully populated
```

before PHASE_1 begins.

After PHASE_1:

```text
cheapest[] is finalized
```

before PHASE_2 begins.

---

## 5.5 Synchronization Summary

| Primitive | Location | Purpose |
|------------|------------|------------|
| `parallel for schedule(static)` | PHASE_0 | Uniform work distribution |
| `parallel for schedule(guided)` | PHASE_1 | Load balancing |
| `__atomic_load_n()` | Atomic Min | Atomic read |
| `__atomic_compare_exchange_n()` | Atomic Min | Lock-free update |
| Packed 64-bit key | PHASE_1 | Single-word reduction |
| Implicit barrier | Between phases | Publish results |
| Sequential merge phase | PHASE_2 | Safe Union-Find updates |

---

# 6. Avoiding Data Races

## Read-Only Find

Normal path-compressing find:

```cpp
while(parent[u] != u) {
    parent[u] = parent[parent[u]];
    u = parent[u];
}
```

writes shared memory.

Instead:

```cpp
int G_find_ro(int u) const {
    while(parent[u] != u)
        u = parent[u];
    return u;
}
```

performs only reads and is therefore safe in parallel.

---

## Disjoint Writes

PHASE_0 writes:

```cpp
comp[u]
cheapest[u]
```

only from iteration `u`.

No synchronization is required.

---

## Why PHASE_2 Remains Sequential

Parallel unions would require:

- Locks, or
- Concurrent Union-Find

for minimal benefit.

The dominant work already resides in PHASE_1.

---

# 7. Why Cycles Cannot Form

Two safeguards prevent cycle creation.

## Deterministic Tie Breaking

The packed key:

```cpp
(weight << 32) | edge_index
```

provides a total ordering.

Equal weights are broken by edge index.

## Re-Find Check

```cpp
if (ult_u == ult_v)
    continue;
```

prevents edges connecting nodes already merged during the same round.

Together these ensure MST correctness.

---

# 8. End-to-End Round Flow

```text
PHASE_0
│
├─ Parallel (static)
├─ Build comp[]
└─ Reset cheapest[]

        ↓ Barrier

PHASE_1
│
├─ Parallel (guided)
└─ Atomic reduction into cheapest[]

        ↓ Barrier

PHASE_2
│
├─ Sequential
├─ Decode edges
├─ Union components
└─ Update MST weight

        ↓

Repeat until component count stops shrinking
```

---

# 9. Compiling and Running

```bash
g++ -O3 -std=c++17 -fopenmp ECL_MST.cpp -o mst
```

Run with:

```bash
OMP_NUM_THREADS=8 ./mst test/internet.egr 0
```

## Notes

- `-fopenmp` enables OpenMP directives.
- `-std=c++17` is required elsewhere in the project.
- `OMP_NUM_THREADS` controls thread count.
- 64-bit CAS is natively supported on modern x86-64 processors.

For benchmarking:

- Fix `OMP_NUM_THREADS`.
- Average multiple runs.
- Ignore first-run cache warmup effects.

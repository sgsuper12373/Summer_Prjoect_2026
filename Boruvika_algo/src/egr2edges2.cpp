// egr2edges.cpp
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
#include <random>
#include "ECLgraph.h"

using namespace std;

// TODO: confirm this matches MAX_WEIGHT in ECL_boruvkas.cpp exactly
static const int MAX_WEIGHT = 50;

void assign_random_weights(ECLgraph& G, int max_weight, uint32_t seed) {
    if (G.edges == 0) {
        G.eweight = NULL;
        return;
    }

    G.eweight = static_cast<int*>(malloc(G.edges * sizeof(*G.eweight)));
    if (G.eweight == NULL) {
        cerr << "ERROR: memory allocation failed while assigning random edge weights\n";
        exit(EXIT_FAILURE);
    }

    fill(G.eweight, G.eweight + G.edges, 0);

    mt19937 generator(seed);
    uniform_int_distribution<int> distribution(1, max_weight);

    for (int u = 0; u < G.nodes; ++u) {
        for (int i = G.nindex[u]; i < G.nindex[u + 1]; ++i) {
            const int v = G.nlist[i];
            if (u > v || G.eweight[i] != 0) continue;

            const int weight = distribution(generator);
            G.eweight[i] = weight;

            if (u == v) continue;

            for (int j = G.nindex[v]; j < G.nindex[v + 1]; ++j) {
                if (G.nlist[j] == u && G.eweight[j] == 0) {
                    G.eweight[j] = weight;
                    break;
                }
            }
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "Usage: %s <input.egr> <output.edges> [weight_seed]\n", argv[0]);
        return -1;
    }

    ECLgraph g = readECLgraph(argv[1]);

    if (g.eweight == NULL) {
        if (argc != 4) {
            fprintf(stderr, "ERROR: %s has no edge weights — a weight_seed argument is required\n", argv[1]);
            freeECLgraph(g);
            return -1;
        }
        uint32_t seed = static_cast<uint32_t>(strtoul(argv[3], NULL, 10));
        assign_random_weights(g, MAX_WEIGHT, seed);
        printf("Note: %s had no weights — generated with seed %u (matching ECL_boruvkas.cpp logic)\n", argv[1], seed);
    }

    FILE* out = fopen(argv[2], "w");
    if (out == NULL) {
        fprintf(stderr, "ERROR: could not open output file %s\n", argv[2]);
        freeECLgraph(g);
        return -1;
    }

    // fprintf(out, "%d %d\n", g.nodes, g.edges); // converter accepted by the galios don't need the header defining number of nodes,edges

    for (int src = 0; src < g.nodes; src++) {
        for (int j = g.nindex[src]; j < g.nindex[src + 1]; j++) {
            int dst = g.nlist[j];
            fprintf(out, "%d %d %d\n", src, dst, g.eweight[j]);
        }
    }

    fclose(out);
    printf("Converted %s: %d nodes, %d edges -> %s\n", argv[1], g.nodes, g.edges, argv[2]);

    freeECLgraph(g);
    return 0;
}
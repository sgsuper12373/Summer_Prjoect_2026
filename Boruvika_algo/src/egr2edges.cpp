// egr2edges.cpp
#include <iostream>   // must come before ECLgraph.h — its printGraphInfo() uses
                      // std::cout but doesn't include <iostream> itself
#include <cstdio>
#include <cstdlib>
#include "ECLgraph.h"

int main(int argc, char* argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.egr> <output.edges>\n", argv[0]);
        return -1;
    }

    ECLgraph g = readECLgraph(argv[1]);

    FILE* out = fopen(argv[2], "w");
    if (out == NULL) {
        fprintf(stderr, "ERROR: could not open output file %s\n", argv[2]);
        return -1;
    }

    fprintf(out, "%d %d\n", g.nodes, g.edges);

    for (int src = 0; src < g.nodes; src++) {
        for (int j = g.nindex[src]; j < g.nindex[src + 1]; j++) {
            int dst = g.nlist[j];
            int wt = (g.eweight != NULL) ? g.eweight[j] : 1;
            fprintf(out, "%d %d %d\n", src, dst, wt);
        }
    }

    fclose(out);
    printf("Converted %s: %d nodes, %d edges -> %s\n", argv[1], g.nodes, g.edges, argv[2]);

    freeECLgraph(g);
    return 0;
}
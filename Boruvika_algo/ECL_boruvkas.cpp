#include<bits/stdc++.h>
#include "ECLgraph.h"
#include "DSU_datastructures.hpp"
#include <chrono>

using namespace std;
using namespace std::chrono;

// Template function for Boruvka's algorithm, accepting the DSU type
template <typename DSU_Type>
int Boruvka_CPU(ECLgraph G) {
    // Instantiate the specified DSU structure
    DSU_Type dsu(G.nodes);
    
    int MST_Weight = 0;
    
    // To be implemented manually: 
    // Core Boruvka logic using dsu.G_find() and dsu.G_union()
    
    return MST_Weight;
}

void print_usage() {
    cerr << "USAGE: ./ecl_boruvkas <filename> <dsu_type>\n"; 
    cerr << "dsu_type can be: full, half, split\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print_usage();
        return 1; 
    }

    ECLgraph G = readECLgraph(argv[1]); 
    string dsu_type = argv[2];

    cout << "\nMST weight for the " << argv[1] << "\n"; 
    cout << "Total Nodes: " << G.nodes << "\nTotal Edges: " << G.edges << "\n"; 
    cout << "DSU Type: " << dsu_type << "\n";
    cout << "--------------------------------------------------\n";

    auto start = high_resolution_clock::now();
    int mst_weight = 0;

    // Dispatch to the correct template instantiation based on command line argument
    if (dsu_type == "full") {
        mst_weight = Boruvka_CPU<DSU_full_cpu>(G);
    } else if (dsu_type == "half") {
        mst_weight = Boruvka_CPU<DSU_half_cpu>(G);
    } else if (dsu_type == "split") {
        mst_weight = Boruvka_CPU<DSU_split_cpu>(G);
    } else {
        cerr << "Error: Unknown DSU type '" << dsu_type << "'. Please choose from: full, half, split.\n";
        freeECLgraph(G);
        return 1;
    }

    auto end = high_resolution_clock::now(); 
    auto duration = duration_cast<microseconds>(end - start); 
    
    cout << "BORUVKA CPU - " << dsu_type << " PATH COMPRESSION\n" ; 
    cout << "\t MST weight: " << mst_weight << "\n"; 
    cout << "\t Computation time: " << duration.count() << " us\n" ; 
    cout << "--------------------------------------------------\n";

    freeECLgraph(G); 
    return 0; 
}
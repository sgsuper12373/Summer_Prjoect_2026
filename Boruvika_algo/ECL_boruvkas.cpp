#include<bits/stdc++.h>
#include "ECLgraph.h"
#include "DSU_datastructures.hpp"
#include <chrono>

using namespace std;
using namespace std::chrono;

/**
 * @brief Used for finding atmoic min of long long int
 * actully for givien implemntaion it is used to compare upper 32 bits and lower 32 bits are used for tie breaking 
 * 
 * @param addr 
 * @param val 
 */
static inline void atomicMinU64( unsigned long long* addr, unsigned long long val ){
    unsigned long long old = __atomic_load_n(addr, __ATOMIC_RELAXED);
    while( val < old &&
           !__atomic_compare_exchange_n(addr, &old, val, false,
                                        __ATOMIC_RELAXED, __ATOMIC_RELAXED)){
        // on CAS failure old is refreshed; loop re-checks val < old
    }
}

// Template function for Boruvka's algorithm, accepting the DSU type
template <typename DSU_Type>
int Boruvka_CPU(ECLgraph G) {
    // Instantiate the specified DSU structure
    DSU_Type dsu(G.nodes);
    
    int MST_Weight = 0;
    int prev_comps = INT_MAX;
    int curr_comps = G.nodes;

    vector<int> comp(G.nodes);     // comp[u] = component id of node u
    vector<int> cheapest(G.nodes); // cheapest[c] = edge index of comp c's cheapest outgoing edge

    while( prev_comps != curr_comps  ){
        prev_comps = curr_comps;

        // PHASE_0: flatten component ids once and reset cheapest
        for( int u = 0 ; u < G.nodes; u++ ){
            comp[u] = dsu.G_find(u);
            cheapest[u] = -1;
        }

        // PHASE_1:  find the cheapest outgoing edge per component
        for( int u = 0 ; u < G.nodes; u++ ){
            int ult_u = comp[u];
            for( int i = G.nindex[u]; i < G.nindex[u+1]; i++ ){
                int v = G.nlist[i]; 
                int w = G.eweight[i];

                if( ult_u == comp[v] ) continue; // same comp, skip

                if( cheapest[ult_u] == -1 || w < G.eweight[ cheapest[ult_u] ] ){
                    cheapest[ult_u] = i ;
                }
            }
        }

        // PHASE_2: merge comps

        for( int u = 0 ; u <  G.nodes; u++ ){
            if( cheapest[u] == -1 ) continue;

            int i = cheapest[u];
            int v = G.nlist[i];
            int w = G.eweight[i];

            int ult_u = dsu.G_find(u);
            int ult_v = dsu.G_find(v);

            // int ult_u = comp[u]; 
            // int ult_v = comp[v]; 

            if( ult_u == ult_v ) continue;

            dsu.G_union(ult_u, ult_v);
            MST_Weight+=w;
            curr_comps--;
        }

    }

    return MST_Weight;
}


template <typename DSU_type> 
int boruvka_omp( ECLgraph G ){
    DSU_type dsu(G.nodes); 
    int MST_Weight = 0; 
    int prev_comps = INT_MAX; 
    int curr_comps = G.nodes; 

    vector<int> comp(G.nodes);
    vector<unsigned long long> cheapest(G.nodes);
    const unsigned long long INF = ~0ULL;

    while( prev_comps != curr_comps  ){
        prev_comps = curr_comps;

        // PHASE_0: flatten component ids and reset cheapest ( read-only find )
        #pragma omp parallel for schedule(static)
        for( int u = 0 ; u < G.nodes; u++ ){
            comp[u] = dsu.G_find(u);
            cheapest[u] = INF;
        }

        // PHASE_1:  find the cheapest outgoing edge per component 
        #pragma omp parallel for schedule(guided)
        for( int u = 0 ; u < G.nodes; u++ ){
            int ult_u = comp[u];
            for( int i = G.nindex[u]; i < G.nindex[u+1]; i++ ){
                int v = G.nlist[i];

                if( ult_u == comp[v] ) continue; // same comp, skip

                unsigned long long key = ((unsigned long long)(unsigned)G.eweight[i] << 32) | (unsigned)i;
                atomicMinU64(&cheapest[ult_u], key);
            }
        }

        // PHASE_2: merge comps

        #pragma omp parallel for schedule(guided)
        for( int c = 0 ; c <  G.nodes; c++ ){
            if( cheapest[c] == INF ) continue;

            int i = (int)(cheapest[c] & 0xffffffffu);
            int w = (int)(cheapest[c] >> 32);
            int v = G.nlist[i];

            if( dsu.G_union(c, v) ) {
                #pragma omp atomic
                MST_Weight+=w; 
                #pragma omp atomic
                curr_comps--; 
            }
        }

    }

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
    }else if (dsu_type == "omp_split"){
        mst_weight = boruvka_omp<DSU_half_omp>(G); 
    } 
    else {
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
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
long long boruvka_omp( ECLgraph G ){
    DSU_type dsu(G.nodes);
    long long MST_Weight = 0;
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
    cerr << "USAGE: ./ecl_boruvkas <filename> [output_csv]\n";
    cerr << "Runs serial (full/half/split) and OMP Boruvka N times each and writes\n";
    cerr << "per-run timings to a CSV (default: boruvka_timings.csv).\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    ECLgraph G = readECLgraph(argv[1]);
    const char* csv_path = (argc >= 3) ? argv[2] : "boruvka_timings.csv";
    const int N_RUNS = 10;

    cout << "\nMST benchmark for " << argv[1] << "\n";
    cout << "Total Nodes: " << G.nodes << "\nTotal Edges: " << G.edges << "\n";
    cout << "OMP threads: " << omp_get_max_threads() << "\n";
    cout << "Runs per version: " << N_RUNS << "\n";
    cout << "--------------------------------------------------\n";

    // Each version: a CSV label and a callable returning the MST weight.
    vector<pair<const char*, function<long long(ECLgraph)>>> methods = {
        { "serial_full",  [](ECLgraph g){ return (long long)Boruvka_CPU<DSU_full_cpu>(g);  } },
        { "serial_half",  [](ECLgraph g){ return (long long)Boruvka_CPU<DSU_half_cpu>(g);  } },
        { "serial_split", [](ECLgraph g){ return (long long)Boruvka_CPU<DSU_split_cpu>(g); } },
        { "omp_half",     [](ECLgraph g){ return boruvka_omp<DSU_half_omp>(g);             } },
    };

    ofstream csv(csv_path);
    if (!csv) {
        cerr << "ERROR: could not open CSV file '" << csv_path << "' for writing\n";
        freeECLgraph(G);
        return 1;
    }
    csv << "version,run,time_us,mst_weight\n";

    // No warm-up: every run is recorded so the median is taken over raw runs
    // (the cold first run is just one of N, and the median ignores it).
    for (auto& m : methods) {
        for (int run = 1; run <= N_RUNS; run++) {
            auto start = high_resolution_clock::now();
            long long weight = m.second(G);
            auto end = high_resolution_clock::now();
            long long us = duration_cast<microseconds>(end - start).count();

            csv << m.first << "," << run << "," << us << "," << weight << "\n";
            cout << left << setw(14) << m.first
                 << "run " << right << setw(2) << run
                 << "  " << setw(9) << us << " us\n";
        }
    }
    csv.close();

    cout << "--------------------------------------------------\n";
    cout << "Wrote " << methods.size() * N_RUNS << " rows to " << csv_path << "\n";
    cout << "CSV columns: version,run,time_us,mst_weight\n";

    freeECLgraph(G);
    return 0;
}
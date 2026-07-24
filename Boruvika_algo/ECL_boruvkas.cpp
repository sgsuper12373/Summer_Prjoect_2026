/**
 * 
 * vary the number of threads => (2,4,8,12,16)  -> DONE
 * schedule => (static -> with atleast 3 diffrent size based on cache size, dyanmic -> try with 2-3 chunk size)
 * 
 * Identify where all reduandant work is done, mainly the parallelized loops
 * Test with some baseline published source code, preferabily for multicore CPU
 * Time different phases of the algorithms. -> Done
 * check how many number of iteration of while loop are being done -> DONE
 * 
 * Performace checks for each phase, study access patterns, 
 * 
 */

#include<bits/stdc++.h>
#include <omp.h>
#include "ECLgraph.h"
#include "DSU_datastructures.hpp"
#include <chrono>
#include <filesystem>

using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;



struct timed_phases{
    double phase0  = 0.0 ; // us: flatten component ids + reset cheapest
    double phase1  = 0.0 ; // us: find cheapest outgoing edge per component
    double phase2  = 0.0 ; // us: merge components
    long long iterations = 0 ; // number of while-loop passes

    void reset_timer(){
        phase0 = 0.0;
        phase1 = 0.0;
        phase2 = 0.0;
        iterations = 0;
    }
};

static timed_phases phase_timer; 

/**
 * @brief Used for finding atmoic min of long long int
 * actully for givien implemntaion it is used to compare upper 32 bits which are used for choosing 
 * minimum outogin edge,  and lower 32 bits are used for tie breaking 
 * 
 * @note
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
int Boruvka_CPU(ECLgraph G )  {
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
            cheapest[u] = INT_MAX;
        }

        // PHASE_1:  find the cheapest outgoing edge per component
        for( int u = 0 ; u < G.nodes; u++ ){
            int ult_u = comp[u];
            for( int i = G.nindex[u]; i < G.nindex[u+1]; i++ ){
                int v = G.nlist[i]; 
                int w = G.eweight[i];

                if( ult_u == comp[v] ) continue; // same comp, skip

                if( cheapest[ult_u] == INT_MAX || w < G.eweight[ cheapest[ult_u] ] ){
                    cheapest[ult_u] = i ;
                }
            }
        }

        // PHASE_2: merge comps

        for( int u = 0 ; u <  G.nodes; u++ ){
            if( cheapest[u] == INT_MAX ) continue;

            int i = cheapest[u];
            int v = G.nlist[i];
            int w = G.eweight[i];

            if(dsu.G_union(u, v)){
                MST_Weight+=w;
                curr_comps--;
            }
        }

    }

    return MST_Weight;
}


template <typename DSU_type>
long long boruvka_omp( ECLgraph G, int chunk_size) {
    DSU_type dsu(G.nodes);
    long long MST_Weight = 0;
    int prev_comps = INT_MAX; 
    int curr_comps = G.nodes; 

    vector<int> comp(G.nodes);
    vector<unsigned long long> cheapest(G.nodes);
    const unsigned long long INF = ~0ULL;

    while( prev_comps != curr_comps  ){
        prev_comps = curr_comps;
        phase_timer.iterations++;

        // PHASE_0: flatten component ids and reset cheapest
        /*
            cache line is 64bytes and I am storing integers in both comp and cheapes array 
            hence one cache line can have 64 / 4 -> 16 elements 
            hence schedule(static,16) would help to reduce the false sharing.....
        */
        auto start = high_resolution_clock::now();
        #pragma omp parallel for schedule(static,chunk_size)
        for( int u = 0 ; u < G.nodes; u++ ){
            comp[u] = dsu.G_find(u);
            cheapest[u] = INF;
        }
        auto end = high_resolution_clock::now(); 
        phase_timer.phase0 += duration_cast<microseconds>(end - start).count();

        

        // PHASE_1:  find the cheapest outgoing edge per component
        /*
            Why Guided suites here best? 
                the work for each thread will be irregualr. based on graph propery we will 
                have the different degree of nodes. some nodes may have higher degree and some might low
                so this is irregular type of work. hence dynamic would work here best I guess because
                dynmic will get more scheduling overhead due to scheduling 
        */
        start = high_resolution_clock::now();
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
        end = high_resolution_clock::now(); 
        phase_timer.phase1 += duration_cast<microseconds>(end - start).count();

        // PHASE_2: merge comps
        // Accumulate via reductions instead of atomics on shared counters.
        long long roundW = 0;
        int merges = 0;

        start = high_resolution_clock::now();
        #pragma omp parallel for schedule(guided) reduction(+:roundW) reduction(+:merges)
        for( int c = 0 ; c <  G.nodes; c++ ){
            if( cheapest[c] == INF ) continue;

            int i = (int)(cheapest[c] & 0xffffffffu);
            int w = (int)(cheapest[c] >> 32);
            int v = G.nlist[i];

            if( dsu.G_union(c, v) ) {
                roundW += w;
                merges++;
            }
        }
        end = high_resolution_clock::now();
        phase_timer.phase2 += duration_cast<microseconds>(end - start).count();

        MST_Weight += roundW;
        curr_comps -= merges;

    }

    return MST_Weight; 
}



void print_usage() {
    cerr << "USAGE: ./ecl_boruvkas <filename> [results_dir] [-n <threads>] [-p0 <chunk_size>]\n";
    cerr << "Runs serial (full/half/split) and OMP Boruvka N times each and writes\n";
    cerr << "per-run timings to <results_dir>/<testfile>_result.csv\n";
    cerr << "(results_dir defaults to '/home/sumitgarad/Documents/GitHub/Summer_Prjoect_2026/Boruvika_algo/Results').\n";
    cerr << "If -p0 is not provided, reads from CHUNK_SIZE environment variable.\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    int num_threads = omp_get_max_threads();
    int chunk_size = 16;
    if (const char* env_p0 = getenv("CHUNK_SIZE")) {
        chunk_size = atoi(env_p0);
    }

    fs::path results_dir = fs::path("/home/sumitgarad/Documents/GitHub/Summer_Prjoect_2026/Boruvika_algo/Results");
    string filename = argv[1];

    for (int i = 2; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-n" && i + 1 < argc) {
            num_threads = atoi(argv[++i]);
        } else if (arg == "-p0" && i + 1 < argc) {
            chunk_size = atoi(argv[++i]);
        } else if (arg[0] != '-') {
            results_dir = fs::path(arg);
        } else {
            cerr << "Unknown argument: " << arg << "\n";
        }
    }

    omp_set_num_threads(num_threads);

    ECLgraph G = readECLgraph(filename.c_str());

    fs::create_directories(results_dir);
    string stem = fs::path(filename).stem().string(); // test file name without dir/extension
    fs::path csv_file = results_dir / (stem + "_result.csv");
    string csv_path_str = csv_file.string();
    const char* csv_path = csv_path_str.c_str();
    const int N_RUNS = 9;

    cout << "\nMST benchmark for " << filename << "\n";
    cout << "Total Nodes: " << G.nodes << "\nTotal Edges: " << G.edges << "\n";
    cout << "OMP threads: " << num_threads << "\n";
    cout << "Chunk size: " << chunk_size << "\n";
    cout << "Runs per version: " << N_RUNS << "\n";
    cout << "--------------------------------------------------\n";

    // Each version: a CSV column label and a callable returning the MST weight.
    // vector<pair<const char*, function<long long(ECLgraph)>>> methods = {
    //     { "serial_full",  [](ECLgraph g){ return (long long)Boruvka_CPU<DSU_full_cpu>(g);  } },
    //     { "serial_half",  [](ECLgraph g){ return (long long)Boruvka_CPU<DSU_half_cpu>(g);  } },
    //     { "serial_split", [](ECLgraph g){ return (long long)Boruvka_CPU<DSU_split_cpu>(g); } },
    //     { "omp_half",     [](ECLgraph g){ return boruvka_omp<DSU_half_omp>(g);             } },
    // };

    vector<pair<const char*, function<long long(ECLgraph)>>> methods = {
        { "serial_half",  [](ECLgraph g){ return (long long)Boruvka_CPU<DSU_half_cpu>(g);  } },
        { "omp_half",     [chunk_size](ECLgraph g){ return boruvka_omp<DSU_half_omp>(g, chunk_size);             } },
    };
    const int M = (int)methods.size();

    // time_us[v][r] = time of version v on run r; weight is the same for every
    // version on a given run (kept once per run for the shared 'weight' column).
    vector<vector<long long>> time_us(M, vector<long long>(N_RUNS, 0));
    vector<long long> run_weight(N_RUNS, 0);

    // Per-run phase breakdown for the OMP version (phases are only timed there).
    struct phase_row { double p0, p1, p2; long long iters; };
    vector<phase_row> omp_phases(N_RUNS, {0, 0, 0, 0});

    // No warm-up: every run is recorded so the median is taken over raw runs
    // (the cold first run is just one of N, and the median ignores it).
    for (int v = 0; v < M; v++) {
        for (int r = 0; r < N_RUNS; r++) {
            phase_timer.reset_timer(); 
            auto start = high_resolution_clock::now();
            long long weight = methods[v].second(G);
            auto end = high_resolution_clock::now();
            long long us = duration_cast<microseconds>(end - start).count();

            time_us[v][r]  = us;
            run_weight[r]  = weight;   // identical across versions
            cout << left << setw(14) << methods[v].first
                 << "run " << right << setw(2) << (r + 1)
                 << "  " << setw(9) << us << " us";

            // Only the OMP version times individual phases (iterations > 0).
            if (phase_timer.iterations > 0) {
                omp_phases[r] = { phase_timer.phase0, phase_timer.phase1,
                                  phase_timer.phase2, phase_timer.iterations };
                double total = phase_timer.phase0 + phase_timer.phase1 + phase_timer.phase2;
                double denom = (total > 0.0) ? total : 1.0;
                cout << "  [iters " << phase_timer.iterations << "]"
                     << " p0 " << setw(8) << (long long)phase_timer.phase0 << " us (" << setw(5) << fixed << setprecision(1) << 100.0 * phase_timer.phase0 / denom << "%)"
                     << " p1 " << setw(8) << (long long)phase_timer.phase1 << " us (" << setw(5) << fixed << setprecision(1) << 100.0 * phase_timer.phase1 / denom << "%)"
                     << " p2 " << setw(8) << (long long)phase_timer.phase2 << " us (" << setw(5) << fixed << setprecision(1) << 100.0 * phase_timer.phase2 / denom << "%)";
                cout.unsetf(ios::fixed);
            }
            cout << "\n";
        }
    }

    // Compute medians
    vector<long long> medians(M);
    for (int v = 0; v < M; v++) {
        vector<long long> sorted_times = time_us[v];
        sort(sorted_times.begin(), sorted_times.end());
        medians[v] = sorted_times[N_RUNS / 2];
    }
    long long final_weight = run_weight[0];

    auto phase_cmp = [](const phase_row& a, const phase_row& b) {
        return (a.p0 + a.p1 + a.p2) < (b.p0 + b.p1 + b.p2);
    };
    vector<phase_row> sorted_phases = omp_phases;
    sort(sorted_phases.begin(), sorted_phases.end(), phase_cmp);
    phase_row median_phase = sorted_phases[N_RUNS / 2];

    bool write_header = true;
    if (fs::exists(csv_file)) write_header = (fs::file_size(csv_file) == 0);
    
    ofstream csv(csv_path, ios::app);
    if (!csv) {
        cerr << "ERROR: could not open CSV file '" << csv_path << "' for writing\n";
        freeECLgraph(G);
        return 1;
    }
    if (write_header) {
        csv << "graph,threads,chunk_size_p0,weight";
        for (auto& m : methods) csv << "," << m.first;
        csv << "\n";
    }
    csv << stem << "," << num_threads << "," << chunk_size << "," << final_weight;
    for (int v = 0; v < M; v++) csv << "," << medians[v];
    csv << "\n";
    csv.close();

    cout << "--------------------------------------------------\n";
    cout << "Appended median result to " << csv_path << "\n";
    cout << "CSV columns: graph,threads,chunk_size_p0,weight,serial_half,omp_half\n";

    // Per-phase timing for the OMP version: <testfile>_phases.csv
    fs::path phase_file = results_dir / (stem + "_phases.csv");
    bool write_phase_header = true;
    if (fs::exists(phase_file)) write_phase_header = (fs::file_size(phase_file) == 0);

    ofstream pcsv(phase_file.string(), ios::app);
    if (pcsv) {
        if (write_phase_header) {
            pcsv << "graph,threads,chunk_size_p0,iterations,phase0_us,phase1_us,phase2_us\n";
        }
        pcsv << stem << "," << num_threads << "," << chunk_size << "," 
             << median_phase.iters << ","
             << (long long)median_phase.p0 << ","
             << (long long)median_phase.p1 << ","
             << (long long)median_phase.p2 << "\n";
        pcsv.close();
        cout << "Appended median OMP phase timings to " << phase_file.string() << "\n";
        cout << "phase0=flatten+reset, phase1=find cheapest edge, phase2=merge comps\n";
    } else {
        cerr << "WARNING: could not open phase CSV '" << phase_file.string() << "' for writing\n";
    }

    freeECLgraph(G);
    return 0;
}
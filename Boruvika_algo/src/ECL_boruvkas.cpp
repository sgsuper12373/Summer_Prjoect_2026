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
 * 
 * Alex fallin 
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
    double phase0  = 0.0 ; // s: flatten component ids + reset cheapest
    double phase1  = 0.0 ; // s: find cheapest outgoing edge per component
    double phase2  = 0.0 ; // s: merge
    double phase3  = 0.0 ; // s: this would be used for manual merge
    long long iterations = 0 ; // number of while-loop passes

    void reset_timer(){
        phase0 = 0.0;
        phase1 = 0.0;
        phase2 = 0.0;
        phase3 = 0.0; 
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
        phase_timer.phase0 += duration<double>(end - start).count();

        

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
        phase_timer.phase1 += duration<double>(end - start).count();

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
        phase_timer.phase2 += duration<double>(end - start).count();

        MST_Weight += roundW;
        curr_comps -= merges;

    }

    return MST_Weight; 
}



template <typename DSU_type>
long long  Boruvka_omp_intermediate( ECLgraph G, int chunk_size){
    DSU_type dsu(G.nodes); 
    long long MST_Weight = 0 ; 
    int prev_comps = INT_MAX; 
    int curr_comps = G.nodes; 

    vector<int> comp(G.nodes); 
    vector<unsigned long long> cheapest(G.nodes); 
    const unsigned long long INF = ~0ULL;

    while( prev_comps != curr_comps ){
        
        prev_comps = curr_comps;
        phase_timer.iterations++;

        // PHASE_0 flatten the components ids and reset the cheapest
        auto start = high_resolution_clock::now(); 
        #pragma omp parallel for schedule(static,chunk_size)
        for( int u = 0 ; u < G.nodes; u++ ){
            comp[u] = dsu.G_find(u); 
            cheapest[u] = INF; 
        }
        auto end = high_resolution_clock::now(); 
        phase_timer.phase0 += duration<double>(end-start).count();


        // PHASE_1 find the cheapest outgoing edge from each component
        start = high_resolution_clock::now(); 
        #pragma omp parallel for schedule(guided) 
        for( int u = 0 ; u < G.nodes; u++ ){
            for( int i = G.nindex[u]; i < G.nindex[u+1]; i++ ){
                int v = G.nlist[i]; 

                if( comp[u] == comp[v] ) continue;

                unsigned long long key = ((unsigned long long)(unsigned)G.eweight[i] << 32) | (unsigned)i;
                atomicMinU64(&cheapest[comp[u]], key);
            }

        }
        end = high_resolution_clock::now(); 
        phase_timer.phase1 += duration<double>(end-start).count();

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
        phase_timer.phase2 += duration<double>(end - start).count();

        MST_Weight += roundW;
        curr_comps -= merges;

        // PHASE_3 flatten the tree after unino operations
        start = high_resolution_clock::now(); 
        // dsu.compress_tree_v2(); 
        dsu.compress_tree_v1(); 
        end = high_resolution_clock::now(); 
        phase_timer.phase3 += duration<double>(end-start).count();

    }
    return MST_Weight; 
}



void print_usage() {
    cerr << "USAGE: ./ecl_boruvkas <filename> -algo <name> [-n <threads>] [-p0 <chunk_size>] [--results-dir <path>]\n";
    cerr << "Algorithms (-algo): serial_full, serial_half, serial_split, omp_half, omp_intermediate\n";
    cerr << "Runs the selected algorithm once and appends results to <results_dir>/<testfile>_result.csv\n";
    cerr << "(results_dir defaults to 'Results/<YYYYMMDD_HHMMSS>').\n";
    cerr << "If -p0 is not provided, reads from CHUNK_SIZE environment variable.\n";
}

string timestamped_results_dir() {
    const auto now = system_clock::to_time_t(system_clock::now());
    tm local_time{};
    localtime_r(&now, &local_time);

    ostringstream timestamp;
    timestamp << put_time(&local_time, "%Y%m%d_%H%M%S");
    return (fs::path("Results") / timestamp.str()).string();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    int num_threads = omp_get_max_threads();
    int chunk_size = 16;
    int MAX_WEIGHT = 50; 

    if (const char* env_p0 = getenv("CHUNK_SIZE")) {
        chunk_size = atoi(env_p0);
    }

    fs::path results_dir = timestamped_results_dir();
    string filename = argv[1];
    string algo_name = "";

    for (int i = 2; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-n" && i + 1 < argc) {
            num_threads = atoi(argv[++i]);
        } else if (arg == "-p0" && i + 1 < argc) {
            chunk_size = atoi(argv[++i]);
        } else if ((arg == "--results-dir" || arg == "-r") && i + 1 < argc) {
            results_dir = fs::path(argv[++i]);
        } else if (arg == "-algo" && i + 1 < argc) {
            algo_name = argv[++i];
        } else {
            cerr << "Unknown or misplaced argument: " << arg << "\n";
            print_usage();
            return 1;
        }
    }

    if (algo_name == "") {
        cerr << "ERROR: -algo <name> is required.\n";
        print_usage();
        return 1;
    }

    omp_set_num_threads(num_threads);

    
    //recored the time taken to load the graph

    auto start = high_resolution_clock::now(); 
    ECLgraph G = readECLgraph(filename.c_str());
    auto end = high_resolution_clock::now(); 
    double G_load_time = duration<double>(end - start).count();



    // according to ECL MST paper when graph is unweighted they are assigning the random weights to the graphs. ( page 6 )
    if (G.eweight == NULL) {
        G.eweight = new int[G.edges];
        for (int u = 0; u < G.nodes; u++) {
            for (int j = G.nindex[u]; j < G.nindex[u+1]; j++) {
                int v = G.nlist[j];
                // Symmetric, deterministic, and pseudo-random
                // Since (u + v) and (u * v) are commutative, the reverse edge gets the exact same weight.
                G.eweight[j] = 1 + ((u + v + (u * v)) % MAX_WEIGHT);
            }
        }
    }

    fs::create_directories(results_dir);
    string stem = fs::path(filename).stem().string(); // test file name without dir/extension
    fs::path csv_file = results_dir / (stem + "_result.csv");
    string csv_path_str = csv_file.string();
    const char* csv_path = csv_path_str.c_str();

    cout << "\nMST benchmark for " << filename << "\n";
    cout << "Total Nodes: " << G.nodes << "\nTotal Edges: " << G.edges << "\n";
    cout << "OMP threads: " << num_threads << "\n";
    cout << "Chunk size: " << chunk_size << "\n";
    cout << "Algorithm: " << algo_name << "\n";
    cout << "--------------------------------------------------\n";

    map<string, function<long long(ECLgraph)>> methods;
    methods["serial_full"] = [](ECLgraph G){ return (long long)Boruvka_CPU<DSU_full_cpu>(G); };
    methods["serial_half"] = [](ECLgraph g){ return (long long)Boruvka_CPU<DSU_half_cpu>(g); };
    methods["serial_split"] = [](ECLgraph g){ return (long long)Boruvka_CPU<DSU_split_cpu>(g); };
    methods["omp_half"] = [chunk_size](ECLgraph g){ return boruvka_omp<DSU_half_omp>(g, chunk_size); };
    methods["omp_intermediate"] = [chunk_size](ECLgraph g){ return Boruvka_omp_intermediate<DSU_intermediate_omp>(g, chunk_size); };

    if (methods.find(algo_name) == methods.end()) {
        cerr << "ERROR: Unknown algorithm '" << algo_name << "'\n";
        print_usage();
        freeECLgraph(G);
        return 1;
    }

    phase_timer.reset_timer(); 
    start = high_resolution_clock::now();
    long long weight = methods[algo_name](G);
    end = high_resolution_clock::now();
    double total_time = duration<double>(end - start).count();

    cout << left << setw(18) << algo_name
         << "  " << fixed << setprecision(6) << setw(10) << total_time << " s";

    if (phase_timer.iterations > 0) {
        double total = phase_timer.phase0 + phase_timer.phase1 + phase_timer.phase2 + phase_timer.phase3;
        double denom = (total > 0.0) ? total : 1.0;
        cout << "  [iters " << phase_timer.iterations << "]"
             << " p0 " << setw(10) << setprecision(6) << phase_timer.phase0 << " s (" << setw(5) << setprecision(1) << 100.0 * phase_timer.phase0 / denom << "%)"
             << " p1 " << setw(10) << setprecision(6) << phase_timer.phase1 << " s (" << setw(5) << setprecision(1) << 100.0 * phase_timer.phase1 / denom << "%)"
             << " p2 " << setw(10) << setprecision(6) << phase_timer.phase2 << " s (" << setw(5) << setprecision(1) << 100.0 * phase_timer.phase2 / denom << "%)"
             << " p3 " << setw(10) << setprecision(6) << phase_timer.phase3 << " s (" << setw(5) << setprecision(1) << 100.0 * phase_timer.phase3 / denom << "%)";
        cout.unsetf(ios::fixed);
    }
    cout << "\n";

    bool write_header = true;
    if (fs::exists(csv_file)) write_header = (fs::file_size(csv_file) == 0);
    
    ofstream csv(csv_path, ios::app);
    if (!csv) {
        cerr << "ERROR: could not open CSV file '" << csv_path << "' for writing\n";
        freeECLgraph(G);
        return 1;
    }
    if (write_header) {
        csv << "graph,algorithm,threads,chunk_size_p0,weight,graph_load_time, total_time_s,iterations,phase0_s,phase1_s,phase2_s,phase3_s\n";
    }
    csv << fixed << setprecision(9);
    csv << stem << "," << algo_name << "," << num_threads << "," << chunk_size << "," << weight << ","
        << G_load_time << ", " 
        << total_time << "," << phase_timer.iterations << "," 
        << phase_timer.phase0 << "," << phase_timer.phase1 << "," 
        << phase_timer.phase2 << "," << phase_timer.phase3 << "\n";
    csv.close();

    cout << "--------------------------------------------------\n";
    cout << "Appended result to " << csv_path << "\n";

    freeECLgraph(G);
    return 0;
}

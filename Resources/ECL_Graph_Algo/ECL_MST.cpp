#include<bits/stdc++.h>
#include "ECLgraph.h"
#include<chrono>
#include<omp.h>

using namespace std;
using namespace std::chrono;

class DSU{
private:    
    int N; 
    vector<int> parent, size; 

    void resetDSU() {
        iota(parent.begin(), parent.end(), 0); 
        fill(size.begin(), size.end(), 1); 
    }

    bool isValidNode( int u ){
        if( u>= N || u < 0 ) return 0; 
        return 1; 
    }

public: 
    DSU( int N ){
        this->N = N ; 
        parent.resize(N);
        size.resize(N); 
        resetDSU();
    }

    int G_find( int u ){
        if( !isValidNode(u)){
            cerr << "ERROR: Invalid node/ node out of range"; 
            exit(1); 
        }

        while(parent[u] != u){
            parent[u] = parent[parent[u]];
            u = parent[u];
        }
        return u;
    }

    // read only DSU find, for no synchronization issue with parallel version of code 
    int G_find_ro( int u ) const {
        while(parent[u] != u){
            u = parent[u];
        }
        return u;
    }

    void G_union( int u, int v ){
        if( !isValidNode(u) || !isValidNode(v)){
            cerr << "ERROR: Invalid node/ node out of range"; 
            exit(1); 
        } 
         int ult_u = G_find(u) ; 
        int ult_v = G_find(v) ;

        // SKIP union if already in component
        if( ult_u == ult_v) {
            return ; 
        }
        
        // JOIN The sizes 
        if( size[ult_u] < size[ult_v]){
            parent[ult_u] = ult_v ;
            size[ult_v] += size[ult_u];
        }else{
            parent[ult_v] = ult_u ;
            size[ult_u] += size[ult_v];
        }

    }
}; 


int ECL_MST_prims( ECLgraph G){

    int MST_Weight = 0; 
    vector<int> parent(G.nodes,-1); 
    vector<int> key(G.nodes,INT_MAX); 
    vector<bool> visited(G.nodes,false);

    priority_queue< pair<int,int>, 
                    vector<pair<int,int>>, 
                    greater<pair<int,int>> > pq; 

    for( int i = 0 ; i < G.nodes; i++ ){
        if( !visited[i]){
            key[i] = 0 ; 
            pq.push({0,i}); 
            while(!pq.empty()){
                auto[w,u] = pq.top(); pq.pop(); 
    
                if(visited[u]) continue; 
                visited[u] = true; 
    
                if(parent[u] != -1 ){
                    MST_Weight += w ; 
                }
    
                for( int j = G.nindex[u]; j < G.nindex[u+1]; j++ ){
                    int v = G.nlist[j]; 
                    int wt = G.eweight[j]; 
    
                    if( !visited[v] && wt < key[v] ){
                        key[v] = wt; 
                        parent[v] = u ; 
                        pq.push({wt,v}); 
                    }
                }
            }
        }
    }

    return MST_Weight ; 
}




static inline void atomicMinU64( unsigned long long* addr, unsigned long long val ){
    unsigned long long old = __atomic_load_n(addr, __ATOMIC_RELAXED);
    while( val < old &&
           !__atomic_compare_exchange_n(addr, &old, val, false,
                                        __ATOMIC_RELAXED, __ATOMIC_RELAXED)){
        // on CAS failure old is refreshed; loop re-checks val < old
    }
}

int  ECL_MST_Boruvika_OMP( ECLgraph G ){

    DSU dsu_g = DSU(G.nodes); 
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
            comp[u] = dsu_g.G_find_ro(u);
            cheapest[u] = INF;
        }

        // PHASE_1:  find the cheapest outgoing edge per component ( lock-free )
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

        for( int c = 0 ; c <  G.nodes; c++ ){
            if( cheapest[c] == INF ) continue;

            int i = (int)(cheapest[c] & 0xffffffffu);
            int w = (int)(cheapest[c] >> 32);
            int v = G.nlist[i];

            int ult_u = dsu_g.G_find(c);
            int ult_v = dsu_g.G_find(v);

            if( ult_u == ult_v ) continue;

            dsu_g.G_union(ult_u, ult_v); 
            MST_Weight+=w; 
            curr_comps--; 
        }
        
    }

    return MST_Weight; 

}





int  ECL_MST_Boruvika_CPU( ECLgraph G ){

    DSU dsu_g = DSU(G.nodes); 
    int MST_Weight = 0; 
    int prev_comps = INT_MAX; 
    int curr_comps = G.nodes; 

    while( prev_comps != curr_comps  ){
        // PHASE_1:  find the cheapest outgoing edge 
        prev_comps = curr_comps;
        vector<pair<int,int>> cheapest(G.nodes, {-1,-1}); // pair storing the {u,i}
        for( int u = 0 ; u < G.nodes; u++ ){
            for( int i = G.nindex[u]; i < G.nindex[u+1]; i++ ){
                int v = G.nlist[i]; 
                int w = G.eweight[i]; 

                int ult_u = dsu_g.G_find(u); 
                int ult_v = dsu_g.G_find(v); 

                if( ult_u == ult_v ) continue; // same comp, skip 

                if(cheapest[ult_u].second == -1 || w <  G.eweight[ cheapest[ult_u].second]){
                    cheapest[ult_u]= {u,i} ; 
                }
            }
        }

        // PHASE_2: merge comps

        for( int c = 0 ; c <  G.nodes; c++ ){
            if( cheapest[c].second == -1 ) continue; 

            int i = cheapest[c].second; 

            // find the u from i 
            int u = cheapest[c].first; 
            if( u == -1 ) {
                cerr <<"ERROR: invalid parent \n"; 
                exit(1); 
            }

            int v = G.nlist[i]; 
            int w = G.eweight[i]; 

            int ult_u = dsu_g.G_find(u); 
            int ult_v = dsu_g.G_find(v); 

            if( ult_u == ult_v ) continue;

            dsu_g.G_union(ult_u, ult_v); 
            MST_Weight+=w; 
            curr_comps--; 
        }
        
    }

    return MST_Weight; 

}


void print_usage(){
    cerr<<"USAGE: ./bfs <filename> <src> \n"; 
}

bool isValidSrc(int src, ECLgraph G){
    if(G.nodes <= src || src < 0 ){
        return false; 
    }

    return true; 
}

int main(int argc, char* argv[]){
    if( argc < 3 ){
        print_usage();
        return 1; 
    }

    ECLgraph G = readECLgraph(argv[1]); 


    int src = atoi(argv[2]); 

    if( !isValidSrc(src, G)) {
        cerr << "ERROR: Source node " << src << " is out of bounds. Must be between 0 and " << G.nodes - 1 << ".\n";
        freeECLgraph(G);
        return 1;
    }
    // printGraphInfo(G); 

    
    auto start = high_resolution_clock::now();
    int MST_boruka_cpu  = ECL_MST_Boruvika_CPU(G); 
    auto end = high_resolution_clock::now(); 
    auto boruvka_cpu_duration = duration_cast<microseconds>(end - start); 

    start = high_resolution_clock::now();
    int MST_prims_cpu = ECL_MST_prims(G); 
    end = high_resolution_clock::now(); 
    auto prims_duration = duration_cast<microseconds>(end-start);

    start = high_resolution_clock::now();
    int MST_boruvka_omp = ECL_MST_Boruvika_OMP(G); 
    end = high_resolution_clock::now(); 
    auto boruvka_omp_duration = duration_cast<microseconds>(end-start);

    cout << "\nMST weight for the " <<   argv[1]<<"\n"; 
    cout << "Total Nodes: " << G.nodes << "\nTotal Edges: " << G.edges << "\n"; 
    cout << "BORUVIKA ALGORITHMS\n" ; 
    cout << "\t MST weight: " << MST_boruka_cpu << "\n"; 
    cout << "\t Computation time: " << boruvka_cpu_duration.count() << "\n" ; 
    cout << "PRIMS ALOGORITHMS\n" ; 
    cout << "\t MST weight: " << MST_prims_cpu<< "\n"; 
    cout << "\t Computation time: " << prims_duration.count() << "\n";  
    cout << "BORUKAS OpenMP ALOGORITHMS\n" ;
    cout << "\t MST weight: " << MST_boruvka_omp<< "\n";
    cout << "\t Computation time: " << boruvka_omp_duration.count() << "\n";   
 


    freeECLgraph(G); 
    return 0; 

}
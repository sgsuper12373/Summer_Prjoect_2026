#include<bits/stdc++.h>
#include "ECLgraph.h"
#include<chrono> 

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
    
    return MST_Weight ; 
}





/**
 * @brief Input is the CSR format. the ECL_MST_Boruvika is supoosed to return the weight of MST as there might exist multiple MST
 * 
 * @param G Graph
 * @return int 
 */
int  ECL_MST_Boruvika_CPU( ECLgraph G ){

    DSU dsu_g = DSU(G.nodes); 
    int MST_Weight = 0; 
    int prev_comps = INT_MAX; 
    int curr_comps = G.nodes; 

    while( prev_comps != curr_comps  ){
        // PHASE_1:  find the cheapest outgoing edge 
        prev_comps = curr_comps;
        vector<int> cheapest(G.nodes, -1 ); 
        for( int u = 0 ; u < G.nodes; u++ ){
            for( int i = G.nindex[u]; i < G.nindex[u+1]; i++ ){
                int v = G.nlist[i]; 
                int w = G.eweight[i]; 

                int ult_u = dsu_g.G_find(u); 
                int ult_v = dsu_g.G_find(v); 

                if( ult_u == ult_v ) continue; // same comp, skip 

                if(cheapest[ult_u] == -1 || w <  G.eweight[ cheapest[ult_u]]){
                    cheapest[ult_u] = i ; 
                }
            }
        }

        // PHASE_2: merge comps

        for( int c = 0 ; c <  G.nodes; c++ ){
            if( cheapest[c] == -1 ) continue; 

            int i = cheapest[c]; 

            // find the u from i 
            int u = -1; 
            for( int tmp = 0; tmp < G.nodes; tmp++ ){
                if( G.nindex[tmp] <= i && i < G.nindex[tmp + 1]){
                    u = tmp; 
                }
            }

            if( u == - 1){
                cerr<<"ERROR: Edges not found!"; 
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
    auto duration = duration_cast<microseconds>(end - start); 

    cout << "\nMST weight for the " <<   argv[1]<<"\n"; 
    cout << "Total Nodes: " << G.nodes << "\nTotal Edges: " << G.edges << "\n"; 
    cout << "MST_boruka_cpu " << MST_boruka_cpu <<"\n"; 
    cout << "Computation time: " << duration.count() << "\n";   


    freeECLgraph(G); 
    return 0; 

}
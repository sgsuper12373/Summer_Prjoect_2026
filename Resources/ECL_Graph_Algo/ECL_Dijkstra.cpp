#include<bits/stdc++.h>
#include "ECLgraph.h"
#include<chrono>

using namespace std; 
using namespace std::chrono; 


vector<int> ECL_Dijkstra(ECLgraph G, int src ){
    vector<int> dist(G.nodes, INT_MAX); 
    priority_queue< pair<int,int>, 
                    vector< pair<int,int>> , 
                    greater<pair<int,int>> > pq; 
    
    dist[src] = 0; 
    pq.push({0,src}); 

    while( !pq.empty()){
        auto [w,u] = pq.top(); pq.pop(); 
        if( w > dist[u] || (dist[u] == INT_MAX)) continue;
        for( int i = G.nindex[u]; i < G.nindex[u+1]; i++ ){
            int v = G.nlist[i];             
            if (dist[v] > dist[u] + G.eweight[i]) {
                dist[v] = dist[u] + G.eweight[i];
                pq.push({dist[v], v});
            }
        }
    }

    return dist; 

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


int main( int argc, char * argv[] ){

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

    printGraphInfo(G); 

    vector<int> dist; 

    auto start = high_resolution_clock::now();
    dist = ECL_Dijkstra(G,src); 
    auto end = high_resolution_clock::now(); 
    auto duration = duration_cast<microseconds>(end - start); 

    cout<<"Execution time: " <<  duration.count() << " us\n"; 
    
    for (int i = 0; i < G.nodes; i++)
    cout << i << ": " << dist[i] << "\n";

    freeECLgraph(G); 
    return 0; 

}
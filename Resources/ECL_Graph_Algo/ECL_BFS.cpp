#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <string>
#include <cstdlib>
#include "ECLgraph.h"

using namespace std; 
using namespace std::chrono;


vector<int> ECL_BFS( ECLgraph& G, int src ){
    vector<int> dist(G.nodes, -1 );
    queue<int> q; 
    q.push(src); 
    dist[src] = 0 ; 

    while(!q.empty()){
        int currNode = q.front(); q.pop(); 

        // discover the neighbours 
        for( int i = G.nindex[currNode]; i < G.nindex[currNode + 1]; i++ ){
            int neighbour = G.nlist[i]; 
            if( dist[neighbour] != -1 ){
                continue;
            }
            dist[neighbour] = dist[currNode] + 1; 
            q.push(neighbour); 
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


int main( int argc , char * argv[]){

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
    dist = ECL_BFS(G,src); 
    auto end = high_resolution_clock::now(); 
    auto duration = duration_cast<microseconds>(end - start); 

    cout<<"Execution time: " <<  duration.count() << " us\n"; 
    
    for (int i = 0; i < G.nodes; i++)
    cout << i << ": " << dist[i] << "\n";

    freeECLgraph(G); 
    return 0; 
}
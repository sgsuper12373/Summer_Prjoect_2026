#include<bits/stdc++.h> 
#include "parser.hpp"

using namespace std; 

vector<int> bfs(int N, int src, const vector<int>& row_ptr, const vector<int>& col_idx){

    vector<bool> visited(N,false); 
    vector<int> dist(N, INT_MAX); 
    queue<int> q; 
    q.push(src); 
    visited[src] = true; 
    dist[src] = 0 ; 

    while( !q.empty()){
        int u = q.front(); q.pop(); 

        for( int i = row_ptr[u]; i < row_ptr[u+1]; i++){
            int v = col_idx[i]; 
            if( !visited[v] ){
                dist[v] = dist[u] + 1 ; 
                visited[v] = true; 
                q.push(v); 
            }
        }
    }

    return dist; 
}

int main( int argc, char * argv[]){
    if( argc < 2 ){
        cerr << " Input File missing\n"; 
        cerr << " Usage: ./a.out <file_name> \n"; 
        return 1; 
    }

    string filename = (string) argv[1]; 
    ifstream file(filename); 

    if (!file) { 
        cerr << "Cannot open file\n"; 
        return 1; 
     }
     
    string line; 
    getline(file, line); 
    stringstream ss(line); 

    int N , NNZ, SRC; 
    ss >> N >> NNZ >> SRC; 

    vector<int> row_ptr, col_idx, values; 
    file_to_csr(file,N, NNZ, row_ptr,col_idx,values);

    vector<int> dist = bfs(N, SRC, row_ptr, col_idx); 

    cout << "\nBFS distances from node " << SRC << ":\n";
    for (int i = 0; i < N; i++)
        cout << "  node " << i << " -> " << (dist[i] == INT_MAX ? "unreachable" : to_string(dist[i])) << "\n";


    

    return 0 ; 
}

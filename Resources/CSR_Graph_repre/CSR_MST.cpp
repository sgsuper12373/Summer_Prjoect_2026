#include<bits/stdc++.h>
#include "parser.hpp"

using namespace std; 


// uses prims algorithm 
vector<pair<int,int>> MST_prim( int N , 
    const vector<int>& row_ptr, 
    const vector<int>& col_idx, 
    const vector<int>& values )
{   
    vector<int>  parent(N, -1); 
    vector<int>  key(N, INT_MAX); 
    vector<bool> visited(N, false); 

    priority_queue<pair<int,int>, 
                   vector<pair<int,int>>, 
                   greater<pair<int,int>>> pq;

    vector<pair<int,int>> MST; 

    // MSF loop handles disconnected graphs
    for( int i = 0; i < N; i++ ){
        if( !visited[i] ){
            key[i] = 0; 
            pq.push({0, i}); 
        }

        while( !pq.empty() ){
            auto [w, u] = pq.top(); pq.pop(); 

            if( visited[u] ) continue;  // stale entry skip
            visited[u] = true; 

            if( parent[u] != -1 )
                MST.push_back({parent[u], u});  // settled edge

            for( int j = row_ptr[u]; j < row_ptr[u+1]; j++ ){
                int v  = col_idx[j]; 
                int wt = values[j]; 

                if( !visited[v] && wt < key[v] ){
                    key[v]    = wt; 
                    parent[v] = u; 
                    pq.push({wt, v}); 
                }
            }
        }
    }
    return MST; 
}


int main( int argc, char* argv[]){
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

    vector<pair<int,int>> MST = MST_prim(N,row_ptr,col_idx, values); 

    cout<< "\n MST for Given Graph is as follow \n"; 
    for( auto it : MST ){
        cout << it.first << "<->" << it.second << " \n"; 
    }

    return 0 ; 
}
#include<bits/stdc++.h>
#include "parser.hpp"


vector<int> dijkstra(int N, int src, 
                     const vector<int>& row_ptr, 
                     const vector<int>& col_idx, 
                     const vector<int>& values)
{
    /*
        I need prriority queue for this. everything works same as BFS. 
    */
    vector<int> dist(N, INT_MAX); 


    // element of proirity queue ->  {wt {u,v}}
    priority_queue< pair<int,int>,
                    vector<pair<int,int>> , 
                    greater<pair<int,int>>> pq; 
    dist[src] = 0 ; 
    pq.push({0,src}); 

    while( !pq.empty()){
        auto [w,u] = pq.top(); pq.pop(); 
        if (w > dist[u]) continue;
        
        for( int i = row_ptr[u];  i < row_ptr[u+1]; i++ ){
            int v = col_idx[i]; 
            
       
            if (dist[v] > dist[u] + values[i]) {
                dist[v] = dist[u] + values[i];
                pq.push({dist[v], v});
            }


        }
    }


    return dist; 
}

int main(int argc, char * argv[]){
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

    vector<int> dist = dijkstra(N, SRC, row_ptr, col_idx, values); 

    cout << "\nDijkstra distances from node " << SRC << ":\n";
    for (int i = 0; i < N; i++)
        cout << "  node " << i << " -> " << (dist[i] == INT_MAX ? "unreachable" : to_string(dist[i])) << "\n";


    

    return 0 ; 


}
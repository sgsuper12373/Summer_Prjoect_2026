#include<bits/stdc++.h>

using namespace std ; 



vector<vector<int>> file_to_edgelist(ifstream& file, int& nnz){
    vector<vector<int>> edges; 
    nnz = 0 ; 
    string line; 
    while( getline( file, line)){
        stringstream ss(line); 
        int u, v , w; 
        ss >> u >> v >> w ; 
        edges.push_back({u,v,w});  
        nnz++; 
    }
    return edges; 

}

void edge_list_to_csr( int N, int nnz , vector<vector<int>>& edges,
    vector<int>& row_ptr, 
    vector<int>& col_idx, 
    vector<int>& values)
{
    row_ptr.resize(N+1,0); 
    col_idx.resize(nnz); 
    values.resize(nnz);  

    // Step 1 — count degree of each node (same as coo_to_csr)
    for (int i = 0; i < nnz; i++)
        row_ptr[edges[i][0] + 1]++;

    // Step 2 — prefix sum → row_ptr now holds start positions
    for (int i = 1; i <= N; i++)
        row_ptr[i] += row_ptr[i - 1];

    // Step 3 — second pass: place each edge using a write-cursor per row
    vector<int> next_free_slot(row_ptr.begin(), row_ptr.end() - 1); // copy row_ptr[0..N-1]

    for (int i = 0; i < nnz; i++) {
        int u = edges[i][0];
        int v = edges[i][1];
        int w = edges[i][2];

        if (u >= N || u < 0 || v >= N || v < 0)
            throw std::out_of_range("Node index exceeds N");

        int slot = next_free_slot[u]++;
        col_idx[slot] = v;
        values[slot]  = w;
    }
}

void edge_list_to_coo( int N , vector<vector<int>>& edges, 
    vector<int>& row_idx, 
    vector<int>& col_idx, 
    vector<int>& values )
{
    /* Edge list will be list of  { u , v , w }*/
    // The edge list might not be always sorted hence we need to sort the Edge list and build the coo representation 
    auto sorted_edges = edges; // local copy — caller's data untouched
    sort(sorted_edges.begin(), sorted_edges.end(), []( const vector<int>& a, const vector<int>&b){
        if( a[0] == b[0] ){
            return a[1] < b[1]; 
        }else{
            return a[0] < b[0]; 
        }
    }); 

    int num_edges = sorted_edges.size(); 
    row_idx.resize( num_edges ); 
    col_idx.resize( num_edges ); 
    values.resize( num_edges); 

    for( int i = 0 ; i < num_edges; i++ ){
        if( sorted_edges[i][0] >= N || sorted_edges[i][0] < 0  || sorted_edges[i][1] >= N || sorted_edges[i][1] < 0  ){
            throw std::out_of_range("Node index exceeds N");
        }
        row_idx[i] = sorted_edges[i][0]; 
        col_idx[i] = sorted_edges[i][1]; 
        values[i] = sorted_edges[i][2]; 
    }
}



void coo_to_csr( int N, int nnz,
                 const vector<int>& row_idx,
                 const vector<int>& col_idx,
                 const vector<int>& coo_values,
                 vector<int>& row_ptr,
                 vector<int>& csr_col,
                 vector<int>& csr_values )
{
    // col_indices and values carry over unchanged
    csr_col    = col_idx;
    csr_values = coo_values;

    // Step 1 — initialize row_ptr to zero
    row_ptr.assign(N + 1, 0);

    // Step 2 — count non-zeros per row
    for (int i = 0; i < nnz; i++)
        row_ptr[row_idx[i] + 1]++;

    // Step 3 — prefix sum → turn counts into start positions
    for (int i = 1; i <= N; i++)
        row_ptr[i] += row_ptr[i - 1];
}


// file shold strictly have edes in (u v w ) format. 
void file_to_coo(ifstream& file, int N, int & nnz, 
    vector<int>& row_idx, 
    vector<int>& col_idx , 
    vector<int>& values)
{
    cout<< "Creating coo format from edgelist"; 
    nnz = 0 ; 
    vector<vector<int>> edges = file_to_edgelist(file,nnz); 
    edge_list_to_coo( N , edges, row_idx, col_idx, values); 
}

/*
    This is older implementatoin to convert file to csr. the flow is as 
    file -> edge_list->coo->csr
*/
void file_to_csr_v1( ifstream& file, int N, int& nnz,
    vector<int>& row_ptr, 
    vector<int>& col_idx, 
    vector<int>& values)
{
    
    cout << "Creating CSR format from edgelist";
    vector<vector<int>> edges = file_to_edgelist(file,nnz);
    vector<int> row_idx_coo, col_idx_coo, values_coo; 
    edge_list_to_coo(N,edges,row_idx_coo, col_idx_coo, values_coo); 
    coo_to_csr( N, nnz, row_idx_coo, col_idx_coo ,values_coo, row_ptr, col_idx, values ); 

}

/*
    Optimized way to create csr from file 
    file->edge_list->csr
 */
void file_to_csr( ifstream& file, int N, int& nnz,
    vector<int>& row_ptr, 
    vector<int>& col_idx, 
    vector<int>& values)
{
    vector<vector<int>> edges = file_to_edgelist(file, nnz);
    edge_list_to_csr(N, nnz, edges, row_ptr, col_idx, values);
}


#include<iostream> 
#include<bits/stdc++.h> 

using namespace std; 


class Graph{
    struct Edge_{
        int u, v, w ; 

        //constructor
        Edge_(int a, int b, int c) {
        u = a; v = b; w = c;
    }
    }; 

    int N_; 
    vector<Edge_> Edge_list_; // tuple storing edge from u -> v with weight w as {u,v,w}; 
    vector<int> parent_, size_ ; // DSU structure for connected components tracking 

public: 

    // CONSTRUCTOR 
    
    Graph( int N, vector<vector<int>>& Edges) {
        N_ = N; 
        parent_.resize( N, 0); // zero based indexing 
        size_.resize( N, 1); 

        for (auto& edge : Edges) {
            // terminate if edges are not in {u,v,w} format
            if (edge.size() != 3) {
                cerr << "Invalid edge format\n";
                exit(1);
            }

            Edge_list_.push_back({edge[0], edge[1], edge[2]}); 
        }

        for( int i = 0 ; i <N ; i++ ){
            parent_[i] = i ; // everyone is parent of itself initially. 
        }
    }

    // Following implementation uses the Half path compression. 
    int G_find( int u ) {

        if( u < 0 || u >= N_) {
            cerr << "Graph Node out of bound\n" << "Upper Bound : " << N_ << "\n" ; 
            exit(1) ; 
        }

        while( parent_[u] != u ) {
            parent_[u] = parent_[parent_[u]]; 
            u = parent_[u]; 
        }
        return u; 
    }

    void G_union( int u, int v ) {

        if( u < 0 || u >= N_ || v < 0 || v >= N_  ) {
            cerr << "Graph Node out of bound\n" << "Upper Bound : " << N_ << "\n" ; 
            exit(1) ; 
        }
        
        // Find the ultimate parents 
        int ult_u = G_find(u) ; 
        int ult_v = G_find(v) ;

        // SKIP union if already in component
        if( ult_u == ult_v) {
            return ; 
        }
        
        // JOIN The sizes 
        if( size_[ult_u] < size_[ult_v]){
            parent_[ult_u] = ult_v ;
            size_[ult_v] += size_[ult_u];
        }else{
            parent_[ult_v] = ult_u ;
            size_[ult_u] += size_[ult_v];
        }

    }


    bool G_isInSameComp( int u, int v){
	    		return G_find(u) == G_find(v) ;
    }

    // return size of component in which u is contained
    int G_sizeOfComp( int u ){
        return size_[ G_find(u) ];
    }

}; 


int main(){

    return 0 ; 
}

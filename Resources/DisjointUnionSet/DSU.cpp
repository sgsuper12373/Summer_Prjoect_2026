#include <iostream>
#include <bits/stdc++.h>

using namespace std; 
/*
 * Size is used as huristic because it helps to track number of nodes in the component
 * Traditional rank Based huristic don't gives this information, It gives the height of which might change after path compression.
 * */
class DSU_FULL {
    int N_ ; 
    vector<int> parent_, size_;

public:
    // constructor
    DSU_FULL(int n) {
        N_ = n ; 
        parent_.resize(n + 1, 0);
        size_.resize(n + 1, 1);
        for (int i = 0; i <= n; i++) {
            parent_[i] = i;
        }
    }

    // return ultimate parent_ 
    int DSU_find(int node) {
        // base case
        if (parent_[node] == node) return node;
    
        return parent_[node] = DSU_find(parent_[node]);;
    }

    

    void DSU_union(int u, int v) {

        if( u > N_  || v > N_ ){
            cout << "Invalid input, max node id can be : " << N_ << " \n"; 
        }
        int ult_u = DSU_find(u);
        int ult_v = DSU_find(v);
	
        // if already in same component the skip procedure 
        if( ult_u == ult_v ) return ; 
        
            if (size_[ult_u] < size_[ult_v]) {
                // join the smaller to big and update the size of big 
                parent_[ult_u]  = ult_v; 
                size_[ult_v] += size_[ult_u]; 
            }else{
                parent_[ult_v] = ult_u; 
                size_[ult_u] += size_[ult_v] ; 
            }
    }


    bool DSU_isInSameComp( int u, int v){
	    		return DSU_find(u) == DSU_find(v) ; 
    }

    // return size of component in which u is contained 
    int DSU_sizeOfComp( int u ){
        int ult_u = DSU_find(u); 
        return size_[ult_u]; 
    }

};

void print_valid_op(){
    cout << "    UNION  U V:      Addes new edge UV to DSU \n" ; 
    cout << "    FIND   U :      Return parent of U contained in DSU data structure \n" ; 
    cout << "    SIZE   U :        Return the size of the component in which U is contained \n" ; 
    cout << "    IsSameComp  U V: Tells if U and V are in the same component \n" ; 
}

void test_DSU_FULL( int N ){
    cout << "DSU FULL path compresssion version selected inputs accepted as follow \n"; 
    print_valid_op(); 
    cout << "Creating DSU  with size:  " << N << "\n"; 
    DSU_FULL ds(N); 
    
    string input; 
    while( getline(cin , input)){
        stringstream ss(input); 
        string cmd;  
        ss >> cmd; 

        if( cmd == "UNION"){
            int u, v ; 
            ss >> u >> v ; 
            cout << "Adding edge " << u << "-" << v << " to DSU \n"; 
            ds.DSU_union(u,v); 
        }else if ( cmd == "FIND"){
            int u ; 
            ss >> u ; 
            cout << "Parent [" << u << "] = " << ds.DSU_find(u) << "\n"; 
        }else if ( cmd == "SIZE"){
            int u ; 
            ss >> u; 
            cout << "Size [" << u << "] : " << ds.DSU_sizeOfComp(u) << "\n"; 
        }else if ( cmd == "IsSameComp"){
            int u , v ; 
            ss >> u >> v ; 
            if( ds.DSU_isInSameComp(u,v)){
                cout << u << "-" << v << " Are In the Same Component \n"; 
            }else{
                cout << u << "-" << v << " Are Not In the Same Component \n"; 
            }
        }
    }
}


void arg_usage(){
    cout << "Run program like ./executable <DSU_VERSION> <N>"; 
    cout << " 1 =>  DSU_FULL"; 
}


int main( int argc, char * argv[] ) {
    if (argc < 3 ){
        arg_usage(); 
    }

    if (string(argv[1]) == "1"){
        test_DSU_FULL( stoi(argv[2]) ); 
    }

    return 0;
}

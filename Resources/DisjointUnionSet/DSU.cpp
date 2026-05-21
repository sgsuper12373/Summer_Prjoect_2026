#include <iostream>
#include <bits/stdc++.h>

using namespace std; 
/*
 * Size is used as huristic because it helps to track number of nodes in the component
 * Traditional rank Based huristic don't gives this information, It gives the height of tree which might change after path compression.
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
    // using vector.at() because it would give out of bound error 
    int DSU_find(int u) {
        // base case
        if( u < N_ ){
            cout << "out of bound node: " << u << "\n max supported size : " << N_ << "\n"; 
            return -1;
        }
        if (parent_.at(u) == u) return u;
    
        return parent_[u] = DSU_find(parent_[u]);
    }

    

    void DSU_union(int u, int v) {

        if( u > N_  || v > N_ ){
            cout << "Invalid input, max node id can be : " << N_ << " \n"; 
            return ; 
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


class DSU_HALF{
    int N_ ; 
    vector<int> parent_, size_ ; 

public: 
    DSU_HALF( int n ){
        N_ = n ; 
        parent_.resize(n+1,0); 
        size_.resize(n+1, 1 ); 
        
        for( int i = 0 ; i <= n ; i++ ){
            parent_[i] = i ; 
        }
    }

    /*
        In Half path compression only DSU_find function will get changed 
        This method is more preffered for GPU as I does very less number of writes compared to other methods 
        Every other node points to it's grandparent 

        @NOTE:  Use .at() instead of [] wherever you want bounds safety. It throws std::out_of_range:
    */ 
    int DSU_find( int u ){

        if( u < N_ ){
            cout << "out of bound node: " << u << "\n max supported size : " << N_ << "\n"; 
            return -1 ;
        }

        while( parent_.at(u) != u  ){
            parent_.at(u) = parent_.at(parent_.at(u));
            u = parent_.at(u); 
        }
        return u; 
    }

    // This will be the same as other methods 
    void DSU_union( int u, int v){
        // if u or v is out of bound we will get error by DSU_find hence no need to worry 
        if( u > N_  || v > N_ ){
            cout << "Invalid input, max node id can be : " << N_ << " \n"; 
            return ; 
        }

        int ult_u = DSU_find(u);  
        int ult_v = DSU_find(v); 

        if( ult_u == ult_v) return ; // same Parent skip the readdition. 

        if( size_[ult_u] < size_[ult_v]){
            parent_[ult_u] = ult_v ; 
            size_[ult_v] += size_[ult_u]; 
        }else{
            parent_[ult_v] = ult_u ; 
            size_[ult_u] += size_[ult_v]; 
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

class DSU_SPLIT{
    int N_; 
    vector<int> parent_, size_ ; 
public: 
    DSU_SPLIT( int N ){
        N_ = N ; 
        parent_.resize(N+1, 0); 
        size_.resize(N+1,1); 
        for( int i = 0 ; i <= N ; i++ ){
            parent_[i] = i; 
        }

    }

    /*
        In the split path compression every node on path gets updated but they don't attach directly to the root.
        Each node points to its grandparent, not one below root
    */

    int DSU_find( int u){

        if( u < N_ ){
            cout << "out of bound node: " << u << "\n max supported size : " << N_ << "\n"; 
            return -1;
        }

        while( parent_[u] != u ){
            int next = parent_.at(u); 
            parent_.at(u) =  parent_.at(parent_.at(u)); // point to grandparent 
            u = next; // move to orginal parent 
        }
        return u; 
    }
    
    void DSU_union( int u , int v){

        if( u > N_  || v > N_ ){
            cout << "Invalid input, max node id can be : " << N_ << " \n"; 
            return ; 
        }

        int ult_u = DSU_find(u); 
        int ult_v = DSU_find(v); 

        if( ult_u == ult_v){ 
            return ; 
        }

        if( size_[ult_u] < size_[ult_v]){
            parent_[ult_u] = ult_v; 
            size_[ult_v] += size_[ult_u]; 
        }else{
            parent_[ult_v] = ult_u; 
            size_[ult_u] += size_[ult_v]; 
        }

    }

    bool DSU_isInSameComp( int u, int v){
        return DSU_find(u) == DSU_find(v); 
    }

    int DSU_sizeOfComp( int u ){
        return size_.at( DSU_find(u)); 
    }

}; 

void print_valid_op(){
    cout << "    UNION  U V:      Addes new edge UV to DSU \n" ; 
    cout << "    FIND   U :      Return parent of U contained in DSU data structure \n" ; 
    cout << "    SIZE   U :        Return the size of the component in which U is contained \n" ; 
    cout << "    IsSameComp  U V: Tells if U and V are in the same component \n" ; 
}


template< typename DSU_TYPE > 
void test_DSU( DSU_TYPE& ds){
    print_valid_op(); 
    
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
        }else{
            cout<< "Invalid input \n"; 
        }
    }
}


template <typename DSUType>
void process_commands(ifstream &file, int N) {
    cout << "Creating DSU with size: " << N << "\n";
    DSUType ds(N);

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string cmd;
        ss >> cmd;

        if (cmd == "UNION") {
            int u, v;
            ss >> u >> v;
            cout << "Adding edge " << u << "-" << v << "\n";
            ds.DSU_union(u, v);

        } else if (cmd == "FIND") {
            int u;
            ss >> u;
            cout << "Parent [" << u << "] = " << ds.DSU_find(u) << "\n";

        } else if (cmd == "SIZE") {
            int u;
            ss >> u;
            cout << "Size [" << u << "] : " << ds.DSU_sizeOfComp(u) << "\n";

        } else if (cmd == "IsSameComp") {
            int u, v;
            ss >> u >> v;
            cout << (ds.DSU_isInSameComp(u, v) ?
                "Same Component\n" : "Not Same Component\n");

        } else {
            cout << "Invalid input\n";
        }
    }
}


void test_file(string file_name) {
    ifstream file(file_name);

    if (!file.is_open()) {
        cout << "Error opening file\n";
        return;
    }

    string line;
    if (!getline(file, line)) {
        cout << "ERROR: UNABLE TO PARSE FILE\n";
        return;
    }

    stringstream ss(line);
    int DSU_varient, N;
    ss >> DSU_varient >> N;

    if (DSU_varient == 1) {
        process_commands<DSU_FULL>(file, N);
    } else if (DSU_varient == 2) {
        process_commands<DSU_HALF>(file, N);
    } else if (DSU_varient == 3) {
        process_commands<DSU_SPLIT>(file, N);
    } else {
        cout << "Invalid DSU variant\n";
    }
}

void arg_usage(){
    cout << "Run program like ./executable <DSU_VERSION> <N>"; 
    cout << " 1 =>  DSU_FULL"; 
    cout << " 2 =>  DSU_HALF"; 
    cout << " 3 =>  DSU_SPLIT"; 
}


int main( int argc, char * argv[] ) {
    if (argc < 3 ){
        arg_usage(); 
        return 1; 
    }

    
    if (string(argv[1]) == "1"){    
        int N = stoi(argv[2]); 
        cout << "Creating DSU  with size:  " << N << "\n"; 
        DSU_FULL ds(N) ; 
        test_DSU( ds ); 
    }else if(string(argv[1]) == "2"){
        int N = stoi(argv[2]); 
        cout << "Creating DSU  with size:  " << N << "\n"; 
        DSU_HALF ds(N); 
        test_DSU( ds ); 
    }else if(string(argv[1]) == "3"){
        int N = stoi(argv[2]); 
        cout << "Creating DSU  with size:  " << N << "\n"; 
        DSU_SPLIT ds(N); 
        test_DSU( ds ); 
    }else if(string(argv[1]) == "-f"){
        string file_name = string(argv[2]); 
        test_file(file_name); 
    }

    return 0;
}

#include<iostream> 
#include<bits/stdc++.h>

using namespace std; 


class DSU_ROLLBACK{
    int N_ ; 
    vector<int> parent_ , size_ ; 
    vector<pair<int*,int>> undo_stack_; 

    bool isValid( int u ){
        if( u < 1 || u > N_){
            cout << " INVALID NODE : " << u << ". Valid Range [ 1, " << N_ << "]\n"; 
            return false; 
        }
        return true; 
    }

public: 

    DSU_ROLLBACK( int N ){
        N_ = N ; 
        parent_.resize(N + 1,0); 
        size_.resize(N + 1,1); 
        for( int i = 0 ; i <= N ; i++ ){
            parent_[i] = i; 
        }
    }


    int DSU_find( int u ){
        if( !isValid(u)) return -1 ;
        while( parent_[u] != u ){
            u = parent_[u]; 
        }

        return u; 
    }

    bool DSU_union( int u, int v ) {
        if( !isValid(u) || !isValid(v) ) return false; 

        int ult_u = DSU_find(u); 
        int ult_v = DSU_find(v); 

        if(ult_u == ult_v){
            undo_stack_.push_back({nullptr, 0}); 
            undo_stack_.push_back({nullptr, 0}); 
            return false; 
        }

        if( size_[ult_u] < size_[ult_v]) swap(ult_u,ult_v); 

        undo_stack_.push_back({&parent_[ult_v], parent_[ult_v]}); // v's parent will change
        undo_stack_.push_back({&size_[ult_u], size_[ult_u]}); // ult_u's size will change
        parent_[ult_v] = ult_u; 
        size_[ult_u] += size_[ult_v]; 

        return true; 
        
    }

    int DSU_sizeOfComp( int u ){
        if( !isValid(u)) return -1; 
        return size_[DSU_find(u)]; 
    }

    bool DSU_isInSameComp( int u, int v ){
        if( !isValid(u) || !isValid(v)) return false; 
        return DSU_find(u) == DSU_find(v) ; 
    }


    //___________________________ROLL_BACK_______________________________________

    int save_checkpoint(){
        return (int) undo_stack_.size(); 
    }

    void rollback_to( int checkpoint ){
        while( (int)undo_stack_.size() != checkpoint ){
            auto& [ptr, old_val] = undo_stack_.back(); 
            undo_stack_.pop_back(); 
            if( ptr ) * ptr = old_val; 
        }
    }

    void rollback_one() {
        if ((int)undo_stack_.size() < 2) {
            cout << "Nothing to rollback\n";
            return;
        }
        rollback_to((int)undo_stack_.size() - 2);
    }

    int history_depth() {
        return (int)undo_stack_.size() / 2;
    }

}; 

void print_valid_op(){
    cout << "DSU ROLLBACK version selected inputs accepted as follow \n";
    cout << "    UNION  U V:      Adds new edge UV to DSU \n" ; 
    cout << "    FIND   U :       Return parent of U contained in DSU data structure \n" ; 
    cout << "    SIZE   U :       Return the size of the component in which U is contained \n" ; 
    cout << "    IsSameComp  U V: Tells if U and V are in the same component \n" ; 
    cout << "    CHECKPOINT:      Saves current state and prints checkpoint ID \n";
    cout << "    ROLLBACK CP:     Rolls back to the specified checkpoint ID \n";
    cout << "    ROLLBACK_ONE:    Rolls back the last union operation \n";
    cout << "    DEPTH:           Returns the history depth (number of union operations) \n";
}

void test_DSU( DSU_ROLLBACK& ds ){
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
        }else if ( cmd == "CHECKPOINT"){
            int cp = ds.save_checkpoint();
            cout << "Checkpoint saved: " << cp << "\n";
        }else if ( cmd == "ROLLBACK"){
            int cp;
            ss >> cp;
            cout << "Rolling back to checkpoint: " << cp << "\n";
            ds.rollback_to(cp);
        }else if ( cmd == "ROLLBACK_ONE"){
            cout << "Rolling back last operation\n";
            ds.rollback_one();
        }else if ( cmd == "DEPTH"){
            cout << "History depth: " << ds.history_depth() << "\n";
        }else{
            cout<< "Invalid input \n"; 
        }
    }
}

void process_commands(ifstream &file, int N) {
    cout << "Creating DSU with size: " << N << "\n";
    DSU_ROLLBACK ds(N);

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
        } else if (cmd == "CHECKPOINT") {
            int cp = ds.save_checkpoint();
            cout << "Checkpoint saved: " << cp << "\n";
        } else if (cmd == "ROLLBACK") {
            int cp;
            ss >> cp;
            cout << "Rolling back to checkpoint: " << cp << "\n";
            ds.rollback_to(cp);
        } else if (cmd == "ROLLBACK_ONE") {
            cout << "Rolling back last operation\n";
            ds.rollback_one();
        } else if (cmd == "DEPTH") {
            cout << "History depth: " << ds.history_depth() << "\n";
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
    int DSU_variant, N;
    if (ss >> DSU_variant) {
        if (ss >> N) {
            // two values: variant and N
        } else {
            // only one value, so it's N
            N = DSU_variant;
        }
    } else {
        cout << "ERROR: UNABLE TO PARSE FILE HEADER\n";
        return;
    }

    process_commands(file, N);
}

void arg_usage(){
    cout << "Run program like ./executable <N>\n"; 
    cout << "Or to read from file: ./executable -f <file_name>\n"; 
}

int main( int argc, char * argv[] ) {
    if (argc < 2 ){
        arg_usage(); 
        return 1; 
    }

    if (string(argv[1]) == "-f"){
        if (argc < 3) {
            arg_usage();
            return 1;
        }
        string file_name = string(argv[2]); 
        test_file(file_name); 
    } else {
        int N;
        if (argc >= 3) {
            N = stoi(argv[2]);
        } else {
            N = stoi(argv[1]);
        }
        cout << "Creating DSU  with size:  " << N << "\n"; 
        DSU_ROLLBACK ds(N); 
        test_DSU( ds ); 
    }

    return 0;
}
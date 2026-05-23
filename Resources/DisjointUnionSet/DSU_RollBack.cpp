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
        parent_.resize(N,0); 
        size_.resize(N,1); 
        for( int i = 0 ; i < N ; i++ ){
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
        parent_[ult_u] = ult_v; 
        size_[ult_u] += size_[ult_v]; 

        return true; 
        
    }

    bool DSU_sizeOfComp( int u ){
        if( !isValid(u)) return -1; 
        return size_[DSU_find(u)]; 
    }

    bool DSU_isInSameComp( int u, int v ){
        if( !isValid(u) || !isValid(v)) return -1; 
        return DSU_find(v) == DSU_find(v) ; 
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



int main(){


}
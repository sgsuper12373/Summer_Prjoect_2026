#pragma once 
#include <bits/stdc++.h>
#include<omp.h> 
using namespace std ;

/**
 * @brief This is the base class for implemetation of DSU
 *        Other version of DSU will be extending this class.
 *        For adding new DSU version just extend this class 
 * 
 */
class DSU{
protected: 
    int N ;
    vector<int> parent; 
    vector<int> size; // size might not used by some versions

    bool isValidNode( int u ){
        if( u < 0 || u >= N ){
            return 0 ; 
        }
        return 1; 
    }

    void resetDSU(){
        iota(parent.begin(), parent.end(),0); 
        fill(size.begin(),size.end(),1); 
    }

public: 
    DSU( int N ){
        this->N = N ; 
        parent.resize(N); 
        size.resize(N); 
        resetDSU(); 
    }

    // Pure virutal fuction, need to be implemented by the subclasses 
    virtual int G_find( int u) = 0 ; 
    virtual bool G_union( int u, int v) = 0 ; 
    virtual bool isInSameComp( int u, int v ) = 0 ; 

    /**
     * @brief Get the Parent array
     * @note array is returned by reference not as copy
     * 
     * @return vector<int>& 
     */
    vector<int>& getParent(){
        return parent; 
    }

    /**
     * @brief Get the size array
     * @note for some implementation which does not use size array this
     * method return array with all values 1. 
     * @note array is returned by reference
     * 
     * @return vector<int>& 
     */
    vector<int>& getSize(){
        return size;
    }


}; 


class DSU_full_cpu: public DSU{
public: 


    DSU_full_cpu( int N ): DSU(N){} 
    

    /**
     * @brief full path compression implemented using the recursion. 
     * @note This can also done by iterative version but I am just keeping it simple recursion
     * @note override is annotation for telling compiler this method is beging overridden. this is common for pure virtual functions
     * @param u 
     * @return int 
     */
    int G_find( int u) override{
        if( !isValidNode(u) ){
            cerr << "Error: invalid / out of bound Node \n"; 
            return -1; 
        }

        if( parent.at(u) == u ) return u; 
        
        return parent[u] = G_find(parent[u]); 

    }


    /**
     * @brief unite the node u and v if they are valid
     *        return true if union is successful else false; 
     * @note we return false if u and v are in same comp to avoid remergeing and adding edge weight again to MST
     * @note but we aree not really uning this return in main code because in main code we are just merging comp if parents are not in same comp
     * @param u 
     * @param v 
     * @return true 
     * @return false 
     */
    bool G_union(int u, int v ) override{
        if( !isValidNode(u)  || !isValidNode(v) ){
            cerr << "Error: invalid / out of bound Node \n"; 
            return 0; 
        }

        int ult_u = G_find(u); 
        int ult_v = G_find(v); 
        
        // if in same comp return false; 
        if( ult_u == ult_v) return 0; 

        if( size[ult_u] > size[ult_v]){
            swap(ult_u, ult_v); 
        }

        parent[ult_u]  = ult_v; 
        size[ult_v] += size[ult_u]; 

        return 1; 

    }


    /**
     * @brief check if u and v are in same component or not.
     * 
     * @param u 
     * @param v 
     * @return true 
     * @return false 
     */
    bool isInSameComp(int u, int v) override{
        return G_find(u) == G_find(v); 
    }

}; 


class DSU_half_cpu: public DSU{
public: 
    DSU_half_cpu( int N ): DSU(N){} 


    /**
     * @brief Find call does the half path compression,  
     *  most parallel frindly version as less writes are done to parent array.
     * 
     * @param u 
     * @return int 
     */
    int G_find( int u) override{
        if( !isValidNode(u) ){
            cerr << "Error: invalid / out of bound Node \n"; 
            return -1; 
        }

        while( parent[u] != u ){
            parent[u] = parent[parent[u]]; 
            u = parent[u]; 
        }
        return u; 
    }


    /**
     * @brief unite the node u and v if they are valid
     *        return true if union is successful else false;
     * @note we return false if both nodes are already in the same comp
     * 
     * @param u 
     * @param v 
     * @return true 
     * @return false 
     */
    bool G_union(int u, int v ) override{
        if( !isValidNode(u)  || !isValidNode(v) ){
            cerr << "Error: invalid / out of bound Node \n"; 
            return 0; 
        }

        int ult_u = G_find(u); 
        int ult_v = G_find(v); 
        
        // if in same comp return false; 
        if( ult_u == ult_v) return 0; 

        if( size[ult_u] > size[ult_v]){
            swap(ult_u, ult_v); 
        }

        parent[ult_u]  = ult_v; 
        size[ult_v] += size[ult_u]; 

        return 1; 

    }


    /**
     * @brief check if u and v are in same component or not.
     * 
     * @param u 
     * @param v 
     * @return true 
     * @return false 
     */
    bool isInSameComp(int u, int v) override{
        return G_find(u) == G_find(v); 
    }
}; 

class DSU_split_cpu: public DSU{
public: 
    DSU_split_cpu(int N): DSU(N){}


    /**
     * @brief Find call does the split path compression,  
     * 
     * @param u 
     * @return int 
     */
    int G_find( int u) override{
        if( !isValidNode(u) ){
            cerr << "Error: invalid / out of bound Node \n"; 
            return -1; 
        }

        while( parent[u] != u ){
            int next = parent[u]; 
            parent[u] = parent[parent[u]]; 
            u = next; 
        }
        return u; 
    }


    /**
     * @brief unite the node u and v if they are valid
     *        return true if union is successful else false;
     * 
     * @param u 
     * @param v 
     * @return true 
     * @return false 
     */
    bool G_union(int u, int v ) override{
        if( !isValidNode(u)  || !isValidNode(v) ){
            cerr << "Error: invalid / out of bound Node \n"; 
            return 0; 
        }

        int ult_u = G_find(u); 
        int ult_v = G_find(v); 
        
        
        if( ult_u == ult_v) return 0; 

        if( size[ult_u] > size[ult_v]){
            swap(ult_u, ult_v); 
        }

        parent[ult_u]  = ult_v; 
        size[ult_v] += size[ult_u]; 

        return 1; 

    }

    /**
     * @brief check if u and v are in same component or not.
     * 
     * @param u 
     * @param v 
     * @return true 
     * @return false 
     */
    bool isInSameComp(int u, int v) override{
        return G_find(u) == G_find(v); 
    }
}; 

class DSU_half_omp: public DSU{
public: 
    DSU_half_omp( int N ) : DSU(N) {}

    int G_find( int u ) override {

        if( !isValidNode(u) ){
            cerr << "Error: invalid Node: " << u << "\n"; 
            exit(1); 
        }

        while(true){
            int p = __atomic_load_n(&parent[u], __ATOMIC_RELAXED);
            if( p == u ) return u;

            int gp = __atomic_load_n(&parent[p], __ATOMIC_RELAXED); // grandparent: path halving
            if( gp == p ) return p; // p is the root

            __atomic_compare_exchange_n(&parent[u],&p, gp, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
            u = gp;
        }
    }


    bool G_union( int u, int v) override {
        if( !isValidNode(u) || !isValidNode(v)){
            cerr << "ERROR: Invalid node/ node out of range"; 
            exit(1); 
        }

        while(true){
            u = G_find(u); 
            v = G_find(v); 

            if( u == v ) return false; 

            if( u > v ) std::swap(u,v); 

            int expected = v ; 
            if (__atomic_compare_exchange_n(&parent[v], &expected, u, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
                return true; // Successfully merged
            }

        }

        return 0 ; 
    }


    bool isInSameComp( int u, int v ) override{
        if( G_find(u) == G_find(v) ){
            return 1; 
        }
        return 0 ; 
    } 
    

}; 

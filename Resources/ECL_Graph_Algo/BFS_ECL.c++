#include<bits/stdc++.h> 
#include "ECLgraph.h"

using namespace std; 




void print_usage(){
    cout<<"USAGE: ./a.out <filename>"; 
}



int main( int argc , char * argv[]){
    if( argc < 2 ){
        cerr << "ERROR: INPUT FILE MISSING" <<"\n";  
        return 1; 
    }

    ECLgraph G = readECLgraph(argv[1]); 

    printGraphInfo(G); 
    

    return 0; 
}
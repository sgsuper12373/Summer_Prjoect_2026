#include<iostream> 
#include<bits/stdc++.h> 
#include "../src/ECLgraph.h"

using namespace std ; 
namespace fs = std::filesystem; 

void storeGraphInfo(const string& path, ofstream& file) {

    ECLgraph G = readECLgraph(path.c_str());

    // main information needed to store 
    int nodes = G.nodes;
    int edges = G.edges;
    bool weighted = (G.eweight != nullptr);

    // compute the size after the extraction which will be loaded into the memory
    size_t sizeAfterExtract =
        2 * sizeof(int) +
        (nodes + 1) * sizeof(int) +
        edges * sizeof(int) +
        (weighted ? edges * sizeof(int) : 0);

    // write the information to the required file 
    file << "# File: " << path << "   \n";
    file << "Nodes: " << nodes << "   \n";
    file << "Edges: " << edges << "   \n";
    file << "Weighted: " << weighted << "    \n";
    file << "Size after extraction: "
         << sizeAfterExtract / (1024.0 * 1024.0)
         << " MB    \n***\n";


    freeECLgraph(G);
}


int main() {
    // current directtory ( can repalce with other directory)
    string path = "./"; 
    string storePath = "./Graphinfo.MD"; 
    ofstream file(storePath);

    if (!file) {
        std::cerr << "Failed to open file\n";
        exit(1);
    }

    // loop throught the current directory and skip the files that are not in .egr format
    for( auto const& entry : fs::directory_iterator(path)){
        string path = entry.path(); 
        cout << path << "\n";

        // check if given file is .egr and then procced to store the information 
        if( path.find(".egr") != string::npos ){
            storeGraphInfo( path, file); 
        }
    }
    return 0  ; 
}
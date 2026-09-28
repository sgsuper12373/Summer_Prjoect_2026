#include <iostream>   // keep before ECLgraph.h (its printGraphInfo uses std::cout)
#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <string>
#include "../src/ECLgraph.h"

using namespace std;
namespace fs = std::filesystem;

static void storeGraphInfo(const fs::path& p, ofstream& csv) {
    ECLgraph G = readECLgraph(p.string().c_str());
    bool weighted = (G.eweight != nullptr);

    csv << p.stem().string() << ',' << G.nodes << ',' << G.edges << ','
        << (weighted ? 1 : 0) << '\n';

    freeECLgraph(G);
}

int main(int argc, char* argv[]) {
    fs::path dir   = (argc > 1) ? argv[1] : "./";
    string outPath = (argc > 2) ? argv[2] : "./graph_info.csv";

    vector<fs::path> files;
    for (auto const& entry : fs::directory_iterator(dir))
        if (entry.is_regular_file() && entry.path().extension() == ".egr")
            files.push_back(entry.path());
    sort(files.begin(), files.end());

    ofstream csv(outPath);
    if (!csv) {
        cerr << "Failed to open " << outPath << "\n";
        return 1;
    }

    csv << "graph,nodes,edges,weighted\n";
    for (auto const& f : files) {
        cout << f << "\n";
        storeGraphInfo(f, csv);
    }
    return 0;
}
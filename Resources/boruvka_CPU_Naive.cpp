#include<bits/stdc++.h> 

using namespace std; 


class Graph{

public: 
    struct Edge_{
        int u, v, w ; 

        //constructor
        Edge_(int a, int b, int c) : u(a), v(b), w(c) {}
    }; 

private: 
    int N_; 
    vector<Edge_> Edge_list_; // tuple storing edge from u -> v with weight w as {u,v,w}; 
    vector<int> parent_, size_ ; // DSU structure for connected components tracking 

    void resetDSU() {
        iota(parent_.begin(), parent_.end(), 0); 
        fill(size_.begin(), size_.end(), 1); 
    }

public: 

    // CONSTRUCTOR 
    
    Graph( int N, const vector<vector<int>>& Edges) {
        N_ = N; 
        parent_.resize(N); // zero based indexing
        size_.resize(N); 

        for (auto& edge : Edges) {
            // terminate if edges are not in {u,v,w} format
            if (edge.size() != 3) {
                cerr << "Invalid edge format\n";
                exit(1);
            }

            Edge_list_.push_back({edge[0], edge[1], edge[2]}); 
        }

        resetDSU();
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

    int G_sizeOfComp( int u ){
        return size_[ G_find(u) ];
    }

    const vector<Edge_>& get_edge_list() const { return  Edge_list_; }

    int get_size() const { return N_; }

}; 


unordered_set<int> boruvka_cpu_naive(int N, const vector<vector<int>>& edges) {
    Graph G(N, edges);
    unordered_set<int> MST;

    const auto& edge_list = G.get_edge_list();
    int prev_components = INT_MAX;
    int curr_components = N ; 

    // to prevent infinite loops in disconnected graphs.
    while ( prev_components != curr_components && curr_components > 1 ) {
        prev_components = curr_components; 
        // cheapest[c] = index of cheapest edge leaving component c
        vector<int> cheapest(N, -1);

        for (int i = 0; i < (int) edge_list.size(); i++) {
            int cu = G.G_find(edge_list[i].u);
            int cv = G.G_find(edge_list[i].v);

            if (cu == cv) continue; // same component, skip

            // For component cu
            if (cheapest[cu] == -1 || edge_list[i].w < edge_list[cheapest[cu]].w)
                cheapest[cu] = i;

            // For component cv
            if (cheapest[cv] == -1 || edge_list[i].w < edge_list[cheapest[cv]].w)
                cheapest[cv] = i;

        }

        // add cheapest edges to MST
        for (int c = 0; c < N; c++) {
            if (cheapest[c] == -1) continue;

            int cu = G.G_find(edge_list[cheapest[c]].u);
            int cv = G.G_find(edge_list[cheapest[c]].v);

            if (cu == cv) continue; // already merged this phase

            G.G_union(cu, cv);
            MST.insert(cheapest[c]);
            curr_components--;
        }
    }

    return MST;
}

void print_valid_op() {
    cout << "Boruvka's MST Algorithm inputs accepted as follows:\n";
    cout << "    ADD_EDGE u v w :  Adds an edge between u and v with weight w\n";
    cout << "    RUN            :  Computes and prints the MST\n";
    cout << "    EXIT           :  Exits the program\n";
}

void process_stream(istream& input, int N, bool interactive) {
    if (interactive) {
        print_valid_op();
    }

    string line;
    vector<vector<int>> edges;
    bool run_called = false;

    while (getline(input, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        
        // Skip comment lines
        ss >> ws;
        if (ss.peek() == 'c') continue;

        string cmd;
        if (isdigit(ss.peek()) || ss.peek() == '-') {
            int u, v, w;
            if (ss >> u >> v >> w) {
                if (interactive) {
                    cout << "Adding edge " << u << "-" << v << " with weight " << w << "\n";
                }
                edges.push_back({u, v, w});
            } else {
                if (interactive) {
                    cout << "Invalid edge format. Expected: u v w\n";
                }
            }
        } else {
            ss >> cmd;
            if (cmd == "ADD_EDGE") {
                int u, v, w;
                if (ss >> u >> v >> w) {
                    if (interactive) {
                        cout << "Adding edge " << u << "-" << v << " with weight " << w << "\n";
                    }
                    edges.push_back({u, v, w});
                } else {
                    if (interactive) {
                        cout << "Invalid edge format. Expected: ADD_EDGE u v w\n";
                    }
                }
            } else if (cmd == "RUN") {
                run_called = true;
                if (edges.empty()) {
                    cout << "No edges added yet.\n";
                    continue;
                }
                unordered_set<int> mst_indices = boruvka_cpu_naive(N, edges);
                vector<vector<int>> mst_edges;
                for (int idx : mst_indices) {
                    mst_edges.push_back(edges[idx]);
                }
                sort(mst_edges.begin(), mst_edges.end());

                long long total_weight = 0;
                cout << "Edges in MST:\n";
                for (const auto& edge : mst_edges) {
                    cout << "  " << edge[0] << " - " << edge[1] << " (weight: " << edge[2] << ")\n";
                    total_weight += edge[2];
                }
                cout << "Total MST Weight: " << total_weight << "\n";
            } else if (cmd == "EXIT") {
                break;
            } else {
                if (interactive) {
                    cout << "Invalid command: " << cmd << "\n";
                }
            }
        }
    }

    // If stream ended without RUN command, run it automatically
    if (!run_called && !edges.empty()) {
        unordered_set<int> mst_indices = boruvka_cpu_naive(N, edges);
        vector<vector<int>> mst_edges;
        for (int idx : mst_indices) {
            mst_edges.push_back(edges[idx]);
        }
        sort(mst_edges.begin(), mst_edges.end());

        long long total_weight = 0;
        cout << "Edges in MST:\n";
        for (const auto& edge : mst_edges) {
            cout << "  " << edge[0] << " - " << edge[1] << " (weight: " << edge[2] << ")\n";
            total_weight += edge[2];
        }
        cout << "Total MST Weight: " << total_weight << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage:\n";
        cerr << "  Interactive: " << argv[0] << " -n <N>\n";
        cerr << "  File input:  " << argv[0] << " -f <file_name>\n";
        return 1;
    }

    string first_arg = argv[1];
    if (first_arg == "-f") {
        if (argc < 3) {
            cerr << "Error: Missing file name for option -f\n";
            return 1;
        }
        string file_name = argv[2];
        ifstream file(file_name);
        if (!file.is_open()) {
            cerr << "Error: Could not open file " << file_name << "\n";
            return 1;
        }

        int N = 0;
        string line;
        if (getline(file, line)) {
            stringstream ss(line);
            string token;
            ss >> token;
            if (token == "-n") {
                ss >> N;
            } else {
                try {
                    N = stoi(token);
                } catch (...) {
                    cerr << "Error: Invalid first line of file. Expected number of nodes.\n";
                    return 1;
                }
            }
        }

        process_stream(file, N, false);
    } else if (first_arg == "-n") {
        if (argc < 3) {
            cerr << "Error: Missing value for option -n\n";
            return 1;
        }
        int N = stoi(argv[2]);
        cout << "Creating Graph with size: " << N << "\n";
        process_stream(cin, N, true);
    } else {
        cerr << "Invalid option: " << first_arg << "\n";
        cerr << "Usage:\n";
        cerr << "  Interactive: " << argv[0] << " -n <N>\n";
        cerr << "  File input:  " << argv[0] << " -f <file_name>\n";
        return 1;
    }

    return 0;
}
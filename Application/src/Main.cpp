
#include "Utils.hpp"
#include "UndirectedGraph.hpp"



int main() {
    string standford_datasets_path = "./datasets/standford";
    stringstream results_path;
    const int initial_n_vertices = 10;
    const int finaal_n_vertices = 1000000;
    const int n_steps = 5;
    
    vector<UndirectedGraph<EdgeList, AdjacentList>> standford_graphs = ReadFromDirectory(standford_datasets_path);
    vector<UndirectedGraph<EdgeList, AdjacentList>> random_graphs[n_steps*2];

    vector<pair<string, map<string, int>>> random_graph_info;

    for(int i = 0; i < n_steps; ++i) {
        Distribution vertex_dist(initial_n_vertices + (i * 10), (initial_n_vertices * 10 + (i * 10) - 1));
        int random_n_vertices = vertex_dist(prng);

        Distribution sparse_dist((random_n_vertices - 1), random_n_vertices * (random_n_vertices - 1) / 4);
        Distribution dense_dist((random_n_vertices * (random_n_vertices - 1) / 4) + 1, random_n_vertices * (random_n_vertices - 1) / 2);

        random_graphs->push_back(UndirectedGraph<EdgeList, AdjacentList>(random_n_vertices, sparse_dist(prng)));
        random_graphs->push_back(UndirectedGraph<EdgeList, AdjacentList>(random_n_vertices, dense_dist(prng)));
        
    }

    // initialization of the results.csv

    time_t result = time(NULL);
    char timestamp[26];
    ctime_s(timestamp, sizeof timestamp, &result);
    string ts = string(timestamp);
    ts.erase(remove(ts.begin(), ts.end(), '\n'), ts.cend());

    replace(ts.begin(), ts.end(), ' ', '_');
    replace(ts.begin(), ts.end(), ':', '.');
    
    results_path << "./results/results_" << ts << ".csv";

    

    

    fstream stream;
    stream.open(results_path.str(), std::ios::out | std::ios::app);
    stream << "name,n_edges,n_vertices,density,threads,n_triangles,elapsed,speed_up\n";
    stream.close();
  
    
    for (auto& graph : standford_graphs) {
        cout << "\n";
        graph.printProprieties();

        cout << "Number of available cores: " << thread::hardware_concurrency() << "\n\n";

        cout << "SEQUENTIAL EXECUTION ...\n";
        graph.SequentialTriangleCounter();
        graph.GetResultByThread(1);

        for (int threads = 2; threads <= static_cast<int>(thread::hardware_concurrency()) * 2; threads++) {
            cout << "PARALLEL EXECUTION WITH " << threads << " THREADS\n";
            graph.ParallelTriangleCounter(threads);
            graph.GetResultByThread(threads);
        }

        graph.WriteResultsCsv(results_path.str());

        cout << "\n\n";


    }


    return 0;
}
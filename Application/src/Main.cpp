
#include "../include/Utils.hpp"
#include "../include/UndirectedGraph.hpp"

constexpr auto MAX_THREADS = 20; // Define the maximum number of thread to run

// Function that made the whole computation start
void RunTriangleCounter(vector<UndirectedGraph<EdgeList, AdjacencyList>> graphs_vector, string results_path) {
    for (auto& graph : graphs_vector) {
        cout << "\n";
        graph.printProprieties();

        cout << "Number of available cores: " << thread::hardware_concurrency() << "\n\n";

        for (int threads = 0; threads < MAX_THREADS; threads++) {

            if (threads == 0)
                cout << "SEQUENTIAL EXECUTION ...\n";
            else
                cout << "PARALLEL EXECUTION WITH " << threads + 1 << " THREADS ...\n";
            graph.TriangleCounter(graph.GetAdjacencyList(threads + 1), threads + 1);
            graph.GetResultByThread(threads);
        }

        graph.WriteResultsCsv(results_path);

        cout << "\n\n";
    }
}



int main() {
    string stanford_datasets_path = "../datasets/stanford";
    string random_datasets_path = "../datasets/random_graphs";
    string results_path;

    bool newer_graph = false;

    cout << "Do you want to generate newer random graphs? (0/1) ";
    cin >> newer_graph;

    if (newer_graph != 0 && newer_graph != 1)
        throw invalid_argument("Please insert correct input");

    if (newer_graph) {
        // Define the range of random graph generation
        vector<pair<int, int>> rages_vector = { {10, 100}, {101, 1000}, {3001, 4100} };

        DeleteExistingDatasets(random_datasets_path);

        // Create the random graphs for the given vector of ranges
        for (auto& ranges : rages_vector) {
            Distribution vertex_dist(ranges.first, ranges.second);
            int random_n_vertices = vertex_dist(prng);

            Distribution sparse_dist((random_n_vertices - 1), random_n_vertices * (random_n_vertices - 1) / 4);
            Distribution dense_dist((random_n_vertices * (random_n_vertices - 1) / 4) + 1, random_n_vertices * (random_n_vertices - 1) / 2);

            GenerateAndWriteSparseRandomGraph(random_datasets_path, random_n_vertices, sparse_dist(prng));
            GenerateAndWriteDenseRandomGraph(random_datasets_path, random_n_vertices, dense_dist(prng));
        }

    }

    results_path = ReturnResultPath();

    fstream stream;
    stream.open(results_path, ios::out | ios::app);
    stream << "name,n_edges,n_vertices,density,threads,n_triangles,elapsed_triangle_count,speed_up_traingle_count,elapsed_adjacency_list,speed_up_adjacency_list\n";
    stream.close();

    RunTriangleCounter(ReadFromDirectory(stanford_datasets_path), results_path);
    RunTriangleCounter(ReadFromDirectory(random_datasets_path), results_path);

    return 0;
}
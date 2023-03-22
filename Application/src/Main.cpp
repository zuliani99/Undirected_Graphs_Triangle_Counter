
#include "../include/Utils.hpp"
#include "../include/UndirectedGraph.hpp"

constexpr auto MAX_THREADS = 20;

void RunTriangleCounter(vector<UndirectedGraph<EdgeList, AdjacentList/*, IncidenteMatrix*/>> graphs_vector, string results_path) {
    for (auto& graph : graphs_vector) {
        cout << "\n";
        graph.printProprieties();

        cout << "Number of available cores: " << thread::hardware_concurrency() << "\n\n";

        //cout << "SEQUENTIAL EXECUTION ...\n";
        //graph.GetAdjacentList(0);
        //graph.SequentialTriangleCounter();
        //graph.GetResultByThread(0);

        for (int threads = 0; threads < MAX_THREADS; threads++) { // untill 20 threads

            if (threads == 0)
                cout << "SEQUENTIAL EXECUTION ...\n";
            else 
                cout << "PARALLEL EXECUTION WITH " << threads + 1 << " THREADS ...\n";
            graph.TriangleCounter(graph.GetAdjacentList(threads + 1), threads + 1);
            //graph.TriangleCounter(graph.GetIncidenteMatrix(threads + 1), threads + 1);
            graph.GetResultByThread(threads);
        }

        graph.WriteResultsCsv(results_path);

        cout << "\n\n";
    }
}


void RunGraphs(string path_datasets, string path_result) {
    RunTriangleCounter(ReadFromDirectory(path_datasets), path_result);
}


int main() {
    string standford_datasets_path = "../datasets/standford";
    string random_datasets_path = "../datasets/random_graphs";
    string results_path;

    bool newer_graph = false;

    //cout << "Do you want to generate newer random graphs? (0/1) ";
    //cin >> newer_graph;

    if (newer_graph != 0 && newer_graph != 1)
        throw std::invalid_argument("Please insert correct input");

    if (newer_graph) {
        const int initial_n_vertices = 10;
        const int finaal_n_vertices = 10000;
        const int n_steps = 3;

        DeleteExistingDatasets(random_datasets_path);

        for (int i = 0; i < n_steps; ++i) {

            Distribution vertex_dist(initial_n_vertices * (static_cast<int>(pow(10, i))), (initial_n_vertices * 10 * static_cast<int>(pow(10, i))) - 1);
            int random_n_vertices = vertex_dist(prng);

            Distribution sparse_dist((random_n_vertices - 1), random_n_vertices * (random_n_vertices - 1) / 4);
            Distribution dense_dist((random_n_vertices * (random_n_vertices - 1) / 4) + 1, random_n_vertices * (random_n_vertices - 1) / 2);

            GenerateAndWriteRandomGraph(random_datasets_path, random_n_vertices, sparse_dist(prng));
            GenerateAndWriteRandomGraph(random_datasets_path, random_n_vertices, dense_dist(prng));

        }
    }

    results_path = ReturnResultPath();

    fstream stream;
    stream.open(results_path, std::ios::out | std::ios::app);
    stream << "name,n_edges,n_vertices,density,threads,n_triangles,elapsed_triangle_count,speed_up_traingle_count,elapsed_adjacent_list,speed_up_adjacent_list\n";
    stream.close();

    RunGraphs(random_datasets_path, results_path);
    //RunGraphs(standford_datasets_path, results_path);

    return 0;
}


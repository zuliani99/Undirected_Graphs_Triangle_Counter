#ifndef _Utils_H
#define _Utils_H

#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <map>
#include <thread>
#include <random>
#include <ctime>
#include <string>
#include <sstream>
#include <filesystem>
#include <cstdio>
#include <omp.h>

using namespace std;


static auto now = chrono::high_resolution_clock::now;
using Duration = chrono::duration<double, milli>;
static mt19937 prng{ random_device{}() };
using Distribution = uniform_int_distribution<int>;

// Defining alias for the two useful data structures
using AdjacencyList = vector<vector<int>>;
using EdgeList = vector<pair<int, int>>;



// Function that given two adjacent lists return the length of the intersection between the two
size_t intersectionLength(vector<int>& adj_l1, vector<int>& adj_l2)
{
    vector<int> intersection;
    set_intersection(adj_l1.begin(), adj_l1.end(), adj_l2.begin(), adj_l2.end(), back_inserter(intersection));
    return intersection.size();
}



// Function that given the path string, the number of vertices and the number of edges, generate a random edges list representing a sparse undirected graph
void GenerateAndWriteSparseRandomGraph(string path, int n_vertices, int n_edges) {
    string name = "V_" + to_string(n_vertices) + "-E_" + to_string(n_edges);
    int first, second;
    vector<pair<int, int>> edges_list;
    ofstream graph;
    Distribution random_vertes(0, n_vertices - 1);


    graph.open(path + "/" + name + ".csv");

    cout << "Generating a random undirected graph with " << n_vertices << " vertices and " << n_edges << " edges ...";

    graph << n_vertices << "\n";


    // For each edge we sample two vertices that correspond to a new graph edge
    for (int edge = 0; edge < n_edges; ++edge) {
        do {
            first = random_vertes(prng);
            second = random_vertes(prng);
        } while (first == second || edges_list.end() != find_if(edges_list.begin(), edges_list.end(),
            [&first, &second](const pair<int, int>& element) { return (element.first == first && element.second == second) ||
            (element.first == second && element.second == first); }));
        // We insert the newer edge only if it meets the above conditions

        edges_list.push_back(make_pair(first, second));

        graph << first << "," << second << "\n";
    }

    graph.close();
    cout << " DONE\n";
}


// Auxiliary function for the function below
bool aux_GenerateAndWriteDenseRandomGraph (pair<int, int> p) {
	return (p.first == 0 && p.second != 0) ||
        	(p.first != 0 && p.second == 0) ||
            (p.first != 0 && p.second != 0);
}


// Function that given the path string, the number of vertices and the number of edges, generate a random edges list representing a sparse undirected graph
void GenerateAndWriteDenseRandomGraph(string path, int n_vertices, int n_edges) {
    string name = "V_" + to_string(n_vertices) + "-E_" + to_string(n_edges);
    int first = 0, second = 0, del = 0, max_threads = thread::hardware_concurrency(), to_del = ((n_vertices * (n_vertices - 1)) / 2) - n_edges, idx_del = 0;
    vector<pair<int, int>> edges_list;
    Distribution random_edges(0, n_edges - 1);
    ofstream graph;

    graph.open(path + "/" + name + ".csv");

    cout << "Generating a random undirected graph with " << n_vertices << " vertices and " << n_edges << " edges ...";

    graph << n_vertices << "\n";

    // Generating all the possible combination of edges in a single direction
    for (int i = 0; i < n_vertices; ++i) {
        for (int j = 0; j < n_vertices; ++j) {
            if (i < j) edges_list.push_back(make_pair(i, j));
        }
    }

    // Marking random edges as deleted
    while (del < to_del) {
        idx_del = random_edges(prng);
        if (aux_GenerateAndWriteDenseRandomGraph(edges_list[idx_del])) {
            edges_list[idx_del].first = 0;
            edges_list[idx_del].second = 0;
            del += 1;
        }
    }

    // Write only the valid edges in the csv file
    for (auto& edge : edges_list) {
        if (aux_GenerateAndWriteDenseRandomGraph(edge))
            graph << edge.first << "," << edge.second << "\n";

    }


    graph.close();
    cout << " DONE\n";
}



// Function to return the string path of the result csv file
string ReturnResultPath() {

    stringstream results_path;

    time_t curtime;
    time(&curtime);
    string ts = string(ctime(&curtime));

    ts.erase(remove(ts.begin(), ts.end(), '\n'), ts.cend());

    replace(ts.begin(), ts.end(), ' ', '_');
    replace(ts.begin(), ts.end(), ':', '.');

    results_path << "../results/results_" << ts << ".csv";
    return results_path.str();
}



// Function to return the path string of the result csv file
void DeleteExistingDatasets(string path) {
	for (const auto& entry : filesystem::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            string path_name = entry.path().generic_string();
            string name = path_name;
            name.erase(name.begin(), name.begin() + 26);

            if (name != ".gitignore")
                remove(path_name.c_str());
        }
    }
}


#endif
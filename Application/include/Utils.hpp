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
# define CHUNK 100

using namespace std;


static auto now = std::chrono::high_resolution_clock::now;
using Duration = std::chrono::duration<double, milli>;
static std::mt19937 prng{ std::random_device{}() };
using Distribution = std::uniform_int_distribution<int>;

using AdjacentList = vector<vector<int>>;
using EdgeList = vector<pair<int, int>>;


size_t intersectionLength(vector<int>& v1, vector<int>& v2)
{
    vector<int> intersection;
    set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(), back_inserter(intersection));
    return intersection.size();
}



void GenerateAndWriteRandomGraph(string path, int n_vertices, int n_edges) {
    string name = "V_" + to_string(n_vertices) + "-E_" + to_string(n_edges);
    int first, second, max_threads = thread::hardware_concurrency();
	int delete_each = (n_vertices*(n_vertices-1) - n_edges) / (n_vertices*(n_vertices-1) / CHUNK);
    //vector<pair<int, int>> edges(n_edges);
    vector<pair<int, int>> edges(n_vertices*(n_vertices-1));

    ofstream graph;
    graph.open(path + "/" + name + ".csv");

    //Distribution random_vertes(0, n_vertices - 1);

    cout << "Generating a random undirected graph with " << n_vertices << " vertices and " << n_edges << " edges ...";

	graph << n_vertices << "\n";

	for(int i = 0; i < n_vertices; ++i) {
		for(int j = 0; j < n_vertices; ++j){
			if(i > j) edges.push_back(make_pair(i, j));
		}
	}

	#pragma omp parallel for schedule(dynamic, CHUNK) num_threads(max_threads) private(delete_each) shared(edges)
	for(int idx = 0; idx < edges.size(); ++idx) {
    	Distribution random_edges(idx, idx + CHUNK);

		for(int del = 0; del < delete_each; ++del){
			int idx_del;
			do {
				idx_del = random_edges(prng);
			}while(edges[idx_del].first == 0 && edges[idx_del].second == 0);
			edges[idx_del].first == 0;
			edges[idx_del].second == 0;
		}
	}

	edges.erase(remove(edges.begin(), edges.end(), make_pair(0, 0)), edges.end());


    /*for (int edge = 0; edge < n_edges; ++edge) {
        do {
            first = random_vertes(prng);
            second = random_vertes(prng);
        } while (first == second || edges.end() != find_if(edges.begin(), edges.end(),
           [&first, &second](const pair<int, int>& element) { return (element.first == first && element.second == second) ||
            (element.first == second && element.second == first); }));

        edges.push_back(make_pair(first, second));

        graph << first << "," << second << "\n";
    }*/

    graph.close();
    cout << " DONE\n";
}


string ReturnResultPath() {

    stringstream results_path;

    time_t curtime;
    time(&curtime);
    string ts = string(ctime(&curtime));

    /*time_t result = time(NULL);
    char timestamp[26];
    ctime_s(timestamp, sizeof timestamp, &result);
    string ts = string(timestamp);*/

    ts.erase(remove(ts.begin(), ts.end(), '\n'), ts.cend());

    replace(ts.begin(), ts.end(), ' ', '_');
    replace(ts.begin(), ts.end(), ':', '.');

    results_path << "../results/results_" << ts << ".csv";
    return results_path.str();
}


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
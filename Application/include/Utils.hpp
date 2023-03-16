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

using namespace std;

static auto now = std::chrono::high_resolution_clock::now;
using Duration = std::chrono::duration<double, milli>;
static std::mt19937 prng{ std::random_device{}() };
using Distribution = std::uniform_int_distribution<int>;

using AdjacentList = unordered_map<int, vector<int>>;
using EdgeList = vector<pair<int, int>>;


size_t intersectionLength(vector<int>* v1, vector<int>* v2)
{   
    vector<int> intersection;
    set_intersection((*v1).begin(), (*v1).end(), (*v2).begin(), (*v2).end(), back_inserter(intersection));
    return intersection.size();
}


size_t TriangleCount(EdgeList *edge_list, AdjacentList *adjacent_list, int start, int skip) { //vector<bool>* sorted
    size_t sum = 0;
    for (int i = start; i < (*edge_list).size(); i += skip) {
        sum += intersectionLength(&(*adjacent_list)[(*edge_list)[i].first], &(*adjacent_list)[(*edge_list)[i].second]);
    }
    return sum;
}


void GenerateAndWriteRandomGraph(int n_vertices, int n_edges) {
	string name = "V_" +to_string(n_vertices) + "-E_" + to_string(n_edges);
    int first, second;
	vector<pair<int, int>> edges(n_edges);

	ofstream graph;
    graph.open ("../datasets/random_graphs/"+name+".csv");

    Distribution random_vertes(0, n_vertices-1);

    for (int edge = 0; edge < n_edges; ++edge) {
        do {
			first = random_vertes(prng);
            second = random_vertes(prng);
        } while (first == second && edges.end() == find_if(edges.begin(), edges.end(),
                [&first, &second](const pair<int, int>& element) { return element.first == first && element.second == second; })); // <-------------------- CONTROLLARE DUPLICATI 

		edges.push_back(make_pair(first, second));

		// added only once
        graph << first << "," << second << "\n";

	}


	/*while (edges.size() < n_edges) {
        first = random_vertes(prng);
        second = random_vertes(prng);
        if (first == second)
            continue; // ignore self-loops
        //if (first > second) {
            //swap(u, v); // make sure u < v
        //}
        if (find(edges.begin(), edges.end(), make_pair(first, second)) == edges.end()) {
			graph << first << "," << second << "\n";
            edges.push_back(make_pair(first, second)); // add the edge if it's not already present
		}
    }*/

    graph.close();

}

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


#endif
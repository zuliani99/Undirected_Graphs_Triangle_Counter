#ifndef _UG_H
#define _UG_H

#include "Utils.hpp"
#include <filesystem>
#include <unordered_map>
#include <string>
#include <sstream>
#include <future>


template<typename EdgeList, typename AdjacentList>
class UndirectedGraph {
private:
    string name = "";
    int n_edges = 0;
    int n_vertices = 0;
    double density = .0;
    //vector<bool> sorted;
    unordered_map<int, double> speed_up;
    unordered_map<int, pair<Duration, size_t>> results;

public:
    EdgeList edge_list;
    AdjacentList adjacent_list;

    void printProprieties();
    void SequentialTriangleCounter();
    void ParallelTriangleCounter(const int num_threads);
    void GetResultByThread(int thread);
    void WriteResultsCsv(string results_path);

    UndirectedGraph() { }

    UndirectedGraph(filesystem::directory_entry entry) {
        if (entry.is_regular_file()) {

            vector<string> row;
            string line, word;

            this->name = entry.path().generic_string();
            this->name.erase(this->name.begin(), this->name.begin() + 11);

            cout << "Reading file: " << this->name << " ...";

            ifstream file(entry.path(), ios::in);

            // Reading the edges file

            if (file.is_open()) {
                while (getline(file, line)) {

                    row.clear();

                    stringstream str(line);

                    while (getline(str, word, ','))
                        row.push_back(word);
                    this->n_edges += 1;

                    this->edge_list.push_back(make_pair(stoi(row[0]), stoi(row[1])));
                }
            }
            file.close();

            for (const auto& edge : this->edge_list) {
                int v1 = edge.first;
                int v2 = edge.second;

                auto it1 = std::upper_bound(this->adjacent_list[v1].cbegin(), this->adjacent_list[v1].cend(), v2);
                this->adjacent_list[v1].insert(it1, v2);

                auto it2 = std::upper_bound(this->adjacent_list[v2].cbegin(), this->adjacent_list[v2].cend(), v1);
                this->adjacent_list[v2].insert(it2, v1);

                //this->adjacent_list[v1].push_back(v2);
                //this->adjacent_list[v2].push_back(v1);
                //this->adjacent_list[v1].insert(v2); // log(N)
                //this->adjacent_list[v2].insert(v1); // log(N)
            }

            this->n_vertices = static_cast<int>(this->adjacent_list.size());   

            //this->sorted = vector<bool>(this->n_vertices, false);
            
            this->density = static_cast<double>((2 * this->n_edges) / (this->n_vertices * (this->n_vertices - 1)));
            cout << "  DONE\n";
        }
    }

    UndirectedGraph(int n_vertices, int n_edges) {
        this->n_vertices = n_vertices;
        this->n_edges = n_edges;
        this->name = to_string(n_vertices) + " " + to_string(n_edges);
        int first, second;

        Distribution random_vertes(0, this->n_vertices);

        for (int edge = 0; edge < this->n_edges; ++edge) {
            first = random_vertes(prng);
            do {
                second = random_vertes(prng);
            } while (first == second && this->edge_list.end() == find_if(this->edge_list.begin(), this->edge_list.end(),
                [&first, &second](const pair<int, int>& element) { return element.first == first && element.second == second; });
            );

            //find_if(this->edge_list.begin(), this->edge_list.end(),
                //[&first, &second](const std::pair<std::string, int>& element) { return element.first == first && element.second == second; });
            this->edge_list.push_back(make_pair(first, second));

            /*auto it1 = upper_bound(this->adjacent_list[first].cbegin(), this->adjacent_list[first].cend(), second);
            this->adjacent_list[first].insert(it1, second);

            auto it2 = upper_bound(this->adjacent_list[second].cbegin(), this->adjacent_list[second].cend(), first);
            this->adjacent_list[second].insert(it2, first);*/
        }

    }

};


template<typename EdgeList, typename AdjacentList>
inline void UndirectedGraph<EdgeList, AdjacentList>::SequentialTriangleCounter()
{
    size_t sum = 0;

    auto start = now();
    sum = TriangleCount(&this->edge_list, &this->adjacent_list, 0, 1); //&this->sorted,
    auto end = now();

    if (sum > 3) sum /= 3;

    this->results[1] = make_pair((end - start), sum);
}


template<typename EdgeList, typename EdgesList>
inline void UndirectedGraph<EdgeList, EdgesList>::ParallelTriangleCounter(int num_threads)
{
    vector<future<size_t>> futures(num_threads);
    size_t sum = 0;

    auto start = now();
    for (int t = 0; t < num_threads; t++) {
        futures[t] = async(launch::async, TriangleCount,
            &this->edge_list, &this->adjacent_list, t, num_threads); //&this->sorted,
    }
    for (int t = 0; t < num_threads; t++) sum += futures[t].get();
    auto end = now();

    if (sum > 0) sum /= 3;

    this->results[num_threads] = make_pair((end - start), sum);
    this->speed_up[num_threads] = this->results[1].first.count() / this->results[num_threads].first.count();
}


template<typename EdgeList, typename AdjacentList>
inline void UndirectedGraph<EdgeList, AdjacentList>::printProprieties()
{
    cout << "Name: " << this->name << " - Number of Edges: " << this->n_edges << " - Number of vertices: " << this->n_vertices << endl << endl;
}


template<typename EdgeList, typename AdjacentList>
inline void UndirectedGraph<EdgeList, AdjacentList>::GetResultByThread(int thread)
{
    cout << "DONE -> " << fixed << "Triangle count: " << this->results[thread].second << " - Execution time : " << this->results[thread].first.count() << "ms - Speed Up: " << this->speed_up[thread] << "\n\n";
}


template<typename EdgeList, typename AdjacentList>
inline void UndirectedGraph<EdgeList, AdjacentList>::WriteResultsCsv(string results_path)
{
    ofstream stream;
    stream.open(results_path, std::ios::out | std::ios::app);
    for (int i = 0; i < this->results.size() - 1; ++i)
        stream << this->name << "," << this->n_edges << "," << this->n_vertices << "," << this->density << "," << (i + 1) << "," << this->results[i + 1].second << "," << this->results[i + 1].first.count() << "," << this->speed_up[i + 1] << "\n";

    stream.close();
}


vector<UndirectedGraph<EdgeList, AdjacentList>> ReadFromDirectory(string path) {
    vector<UndirectedGraph<EdgeList, AdjacentList>> graphs_list;

    for (const auto& entry : filesystem::directory_iterator(path))
        graphs_list.push_back(UndirectedGraph<EdgeList, AdjacentList>(entry));

    return graphs_list;

}


#endif
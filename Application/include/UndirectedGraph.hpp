#ifndef _UG_H
#define _UG_H

#include "Utils.hpp"
#include <unordered_map>
#include <omp.h>
#include <mutex>


template<typename EdgeList, typename AdjacentList>
class UndirectedGraph {
private:
    string name = "";
    int n_edges = 0;
    int n_vertices = 0;
    double density = .0;
    unordered_map<int, Duration> elp_count;
    unordered_map<int, Duration> elp_adj;
    unordered_map<int, size_t> tri_count;
    vector<omp_lock_t> writelock;


public:
    EdgeList edge_list;
    AdjacentList adjacent_list;

    void printProprieties();
    void TriangleCounter(const int num_threads);
    void GetResultByThread(int thread);
    void WriteResultsCsv(string results_path);
    void GetAdjacentList(const int num_threads);

    UndirectedGraph() { }

    UndirectedGraph(filesystem::directory_entry entry) {
        if (entry.is_regular_file()) {

            vector<string> row;
            string line, word;
            vector<int> temp_vertices;

            this->name = entry.path().generic_string();
            this->name.erase(this->name.begin(), this->name.begin() + 11);

            cout << "Reading file: " << this->name << " ...";

            ifstream file(entry.path(), ios::in);

            // Reading the edges file

            if (file.is_open()) {
                while (getline(file, line)) {
                    int first, second;

                    row.clear();

                    stringstream str(line);

                    while (getline(str, word, ','))
                        row.push_back(word);
                    this->n_edges += 1;

                    first = stoi(row[0]);
                    second = stoi(row[1]);

                    this->edge_list.push_back(make_pair(first, second));

                    if (find(temp_vertices.begin(), temp_vertices.end(), first) == temp_vertices.end()) temp_vertices.push_back(first);
                    if (find(temp_vertices.begin(), temp_vertices.end(), second) == temp_vertices.end()) temp_vertices.push_back(second);

                }
            }
            file.close();

            this->n_vertices = static_cast<int>(temp_vertices.size());
            this->writelock = vector<omp_lock_t>(this->n_vertices);

            cout << "  DONE\n";
        }
    }
};



template<typename EdgeList, typename AdjacentList>
inline void UndirectedGraph<EdgeList, AdjacentList>::printProprieties()
{
    cout << "Name: " << this->name << " - Number of Edges: " << this->n_edges << " - Number of vertices: " << this->n_vertices << endl << endl;
}


template<typename EdgeList, typename AdjacentList>
inline void UndirectedGraph<EdgeList, AdjacentList>::TriangleCounter(const int num_threads)
{
    size_t sum = 0;

    auto start = now();

    #pragma omp parallel for if(num_threads>1) num_threads(num_threads) reduction(+:sum)
    for (int i = 0; i < this->n_edges; ++i) {
        sum += intersectionLength(this->adjacent_list[this->edge_list[i].first], this->adjacent_list[this->edge_list[i].second]);
    }

    auto end = now();

    if (sum > 3) sum /= 3;

    this->elp_count[num_threads-1] = (end - start);
    this->tri_count[num_threads-1] = sum;


}


template<typename EdgeList, typename AdjacentList>
inline void UndirectedGraph<EdgeList, AdjacentList>::GetResultByThread(int thread)
{
    cout << "--- DONE ---\n" << fixed << "Triangle count: " << this->tri_count[thread] << " - Execution time : " << this->elp_count[thread].count() << "ms - Speed Up: " << this->elp_count[0].count() / this->elp_count[thread].count() <<
        " - Adjacent List Generation Time: " << this->elp_adj[thread].count() << "ms - Speed Up: " << this->elp_adj[0].count() / this->elp_adj[thread].count() << "\n\n";
}


template<typename EdgeList, typename AdjacentList>
inline void UndirectedGraph<EdgeList, AdjacentList>::WriteResultsCsv(string results_path)
{

    ofstream stream;
    stream.open(results_path, std::ios::out | std::ios::app);
    for (int i = 0; i < this->elp_count.size(); ++i) {
                    //"name,             n_edges,                  n_vertices,            density,                threads
        stream << this->name << "," << this->n_edges << "," << this->n_vertices << "," << this->density << "," << (i + 1) << "," <<
            //    n_triangles,            elapsed_triangle_count,                           speed_up_traingle_count
            this->tri_count[i] << "," << this->elp_count[i].count() << "," << this->elp_count[0].count() / this->elp_count[i].count() << "," <<
            //elapsed_adjacent_list,                         speed_up_adjacent_list\n";
            this->elp_adj[i].count() << "," << this->elp_adj[0].count() / this->elp_adj[i].count() << "\n";

    }
        

    stream.close();
}

template<typename EdgeList, typename AdjacentList>
inline void UndirectedGraph<EdgeList, AdjacentList>::GetAdjacentList(const int num_threads)
{

	// ----------------- ALL THREE THROWN AN EXCEPTION -----------------
    //this->adjacent_list.clear();
    //AdjacentList(this->n_vertices).swap(this->adjacent_list);
    //this->adjacent_list = AdjacentList(this->n_vertices);

    if (num_threads > 1) {
        #pragma omp for
        for(int i = 0; i < this->n_vertices; ++i)
            omp_init_lock(&this->writelock[i]);
    }
    
    auto start = now();

    #pragma omp parallel for if(num_threads > 1) num_threads(num_threads)
    for (int i = 0; i < this->n_edges; i++) {

        int v1 = this->edge_list[i].first;
        int v2 = this->edge_list[i].second;

        if (num_threads > 1) omp_set_lock(&this->writelock[v1]);
        auto it1 = upper_bound(this->adjacent_list[v1].begin(), this->adjacent_list[v1].end(), v2);
        this->adjacent_list[v1].insert(it1, v2);
        if (num_threads > 1) omp_unset_lock(&this->writelock[v1]);

        
        if (num_threads > 1) omp_set_lock(&this->writelock[v2]);
        auto it2 = upper_bound(this->adjacent_list[v2].begin(), this->adjacent_list[v2].end(), v1);
        this->adjacent_list[v2].insert(it2, v1);
        if (num_threads > 1) omp_unset_lock(&this->writelock[v2]);

    }

    auto end = now();

    if (num_threads > 1) {
        #pragma omp for
        for (int i = 0; i < this->n_vertices; ++i)
            omp_destroy_lock(&this->writelock[i]);
    }

    this->elp_adj[num_threads - 1] = (end - start);

    this->density = static_cast<double>((2 * this->n_edges) / (this->n_vertices * (this->n_vertices - 1)));
    
}


vector<UndirectedGraph<EdgeList, AdjacentList>> ReadFromDirectory(string path) {
    vector<UndirectedGraph<EdgeList, AdjacentList>> graphs_list;

    for (const auto& entry : filesystem::directory_iterator(path))
        graphs_list.push_back(UndirectedGraph<EdgeList, AdjacentList>(entry));

    return graphs_list;

}


#endif
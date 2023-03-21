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


public:
    EdgeList edge_list;

    void printProprieties();
    void TriangleCounter(AdjacentList adjacent_list, const int num_threads);
    void GetResultByThread(const int thread);
    void WriteResultsCsv(string results_path);
    AdjacentList GetAdjacentList(const int num_threads);
    //IncidenteMatrix GetIncidenteMatrix(const int num_threads);

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
                    /*auto it2 = lower_bound(this->edge_list.begin(), this->edge_list.end(), make_pair(first, second),
                        [](const std::pair<int, int>& lhs, const std::pair<int, int>& rhs) {
                            return lhs.first < rhs.first;
                        });
                    this->edge_list.insert(it2, make_pair(first, second));
                    */

                    if (find(temp_vertices.begin(), temp_vertices.end(), first) == temp_vertices.end()) temp_vertices.push_back(first);
                    if (find(temp_vertices.begin(), temp_vertices.end(), second) == temp_vertices.end()) temp_vertices.push_back(second);

                }
            }
            file.close();

            sort(this->edge_list.begin(), this->edge_list.end());

            this->n_vertices = static_cast<int>(temp_vertices.size());

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
inline void UndirectedGraph<EdgeList, AdjacentList>::TriangleCounter(AdjacentList adjacent_list, const int num_threads)
{
    size_t sum = 0;

    auto start = now();

#pragma omp parallel for if(num_threads>1) num_threads(num_threads) reduction(+:sum)
    for (int i = 0; i < this->n_edges; ++i) {
        sum += intersectionLength(adjacent_list[this->edge_list[i].first], adjacent_list[this->edge_list[i].second]);
    }

    auto end = now();

    if (sum > 3) sum /= 3;

    this->elp_count[num_threads - 1] = (end - start);
    this->tri_count[num_threads - 1] = sum;


}


template<typename EdgeList, typename AdjacentList>
inline void UndirectedGraph<EdgeList, AdjacentList>::GetResultByThread(const int thread)
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

void merge_map(unordered_map<int, vector<int>>& inout, unordered_map<int, vector<int>>& in) {
    for (auto initer = in.begin(), outiter = inout.begin(); initer != in.end(); ++initer, ++outiter) {
        vector<int> res;
        std::merge(initer->second.begin(), initer->second.end(),
            outiter->second.begin(), outiter->second.end(),
            back_inserter(res));

        auto pte = unique(res.begin(), res.end());
        res.erase(pte, res.end());

        outiter->second = res;
    }
}

#pragma omp declare reduction(merge_map : unordered_map<int, vector<int>> : merge_map(omp_out, omp_in)) initializer (omp_priv=omp_orig)
template<typename EdgeList, typename AdjacentList>
inline AdjacentList UndirectedGraph<EdgeList, AdjacentList>::GetAdjacentList(const int num_threads)
{
    AdjacentList adjacent_list{};
    int v1 = 0, v2 = 0;

    auto start = now();

    #pragma omp parallel for if(num_threads > 1) num_threads(num_threads) reduction(merge_map : adjacent_list)
    for (int i = 0; i < this->n_edges; ++i) {

        v1 = this->edge_list[i].first;
        v2 = this->edge_list[i].second;

        auto it1 = lower_bound(adjacent_list[v1].begin(), adjacent_list[v1].end(), v2);
        adjacent_list[v1].insert(it1, v2);

        auto it2 = lower_bound(adjacent_list[v2].begin(), adjacent_list[v2].end(), v1);
        adjacent_list[v2].insert(it2, v1);

    }

    auto end = now();

    this->elp_adj[num_threads - 1] = (end - start);

    this->density = static_cast<double>((2 * this->n_edges) / (this->n_vertices * (this->n_vertices - 1)));

    return adjacent_list;
}
/*{
    AdjacentList adjacent_list(this->n_vertices);
    vector<omp_lock_t> writelocks(this->n_vertices);
    vector<int> temp(this->n_vertices, 0);
    unordered_map<int, pair<int, int>> offset(this->n_vertices);
    int prev = 0;

    auto start = now();

    if (num_threads > 1) {
        #pragma omp parallel for num_threads(num_threads)
        for (int i = 0; i < this->n_vertices; ++i) {
            omp_init_lock(&writelocks[i]);
        }
    }
    for (int i = 0; i < this->edge_list.size(); ++i) 
temp[this->edge_list[i].first] += 1;
    for (int i = 0; i < temp.size(); ++i) {
        offset[i] = make_pair(prev, prev + temp[i]);
        prev += temp[i];
    }
  
    #pragma omp parallel for if (num_threads > 1) num_threads(num_threads)
    for (int i = 0; i < offset.size(); ++i) {
        auto& [first, second] = offset[i];
        for (int j = first; j < second; ++j) {
            if (num_threads > 1) omp_set_lock(&writelocks[first]);
            auto it1 = lower_bound(adjacent_list[first].begin(), adjacent_list[first].end(), second);
            adjacent_list[first].insert(it1, second);
            if (num_threads > 1) omp_unset_lock(&writelocks[first]);

            if (num_threads > 1) omp_set_lock(&writelocks[second]);
            auto it2 = lower_bound(adjacent_list[second].begin(), adjacent_list[second].end(), first);
            adjacent_list[second].insert(it2, first);
            if (num_threads > 1) omp_unset_lock(&writelocks[second]);
        }
    }

    if (num_threads > 1) {
        #pragma omp parallel for num_threads(num_threads)
        for (int i = 0; i < this->n_vertices; ++i) {
            omp_destroy_lock(&writelocks[i]);
        }
    }

    auto end = now();

    this->elp_adj[num_threads - 1] = (end - start);

    this->density = static_cast<double>((2 * this->n_edges) / (this->n_vertices * (this->n_vertices - 1)));

    return adjacent_list;
}*/
/*{
    AdjacentList adjacent_list{};

    vector<omp_lock_t> writelocks(this->n_vertices);
    std::unordered_map<int, int> vertex_to_lock_map;
    int v1 = 0, v2 = 0, lock_id_1 = 0, lock_id_2 = 0;

    if (num_threads > 1) {
        #pragma omp parallel for num_threads(num_threads)
        for (int i = 0; i < this->n_vertices; ++i) {
            omp_init_lock(&writelocks[i]);
            vertex_to_lock_map[i] = std::hash<int>()(i) % num_threads;
        }
    }

    auto start = now();

    #pragma omp parallel for if(num_threads > 1) num_threads(num_threads) shared(adjacent_list)
    for (int i = 0; i < this->n_edges; ++i) {

        v1 = this->edge_list[i].first;
        v2 = this->edge_list[i].second;

        lock_id_1 = vertex_to_lock_map[v1];
        lock_id_2 = vertex_to_lock_map[v2];

        if (num_threads > 1) omp_set_lock(&writelocks[v1]);
        auto it1 = lower_bound(adjacent_list[v1].begin(), adjacent_list[v1].end(), v2);
        adjacent_list[v1].insert(it1, v2);
        if (num_threads > 1) omp_unset_lock(&writelocks[v1]);

        if (num_threads > 1) omp_set_lock(&writelocks[v2]);
        auto it2 = lower_bound(adjacent_list[v2].begin(), adjacent_list[v2].end(), v1);
        adjacent_list[v2].insert(it2, v1);
        if (num_threads > 1) omp_unset_lock(&writelocks[v2]);

    }

    auto end = now();

    if (num_threads > 1) {
        #pragma omp parallel for num_threads(num_threads)
        for (int i = 0; i < this->n_vertices; ++i) {
            omp_destroy_lock(&writelocks[i]);
        }
    }



    this->elp_adj[num_threads - 1] = (end - start);

    this->density = static_cast<double>((2 * this->n_edges) / (this->n_vertices * (this->n_vertices - 1)));

    return adjacent_list;
}*/

/*template<typename EdgeList, typename AdjacentList>
inline IncidenteMatrix UndirectedGraph<EdgeList, AdjacentList>::GetIncidenteMatrix(const int num_threads)
{
    IncidenteMatrix incidente_matrix(this->n_vertices, vector<bool>(this->n_vertices, false));

    #pragma omp parallel for if(num_threads > 1) num_threads(num_threads) shared(incidente_matrix)
    for (int i = 0; i < this->n_edges; ++i) {
        int v1 = this->edge_list[i].first;
        int v2 = this->edge_list[i].second;

        incidente_matrix[v1][v2] = true;
        incidente_matrix[v2][v1] = true;
    }

    return incidente_matrix
}*/


vector<UndirectedGraph<EdgeList, AdjacentList>> ReadFromDirectory(string path) {
    vector<UndirectedGraph<EdgeList, AdjacentList>> graphs_list;

    for (const auto& entry : filesystem::directory_iterator(path))
        graphs_list.push_back(UndirectedGraph<EdgeList, AdjacentList>(entry));

    return graphs_list;

}


#endif
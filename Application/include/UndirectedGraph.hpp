#ifndef _UG_H
#define _UG_H

#include "Utils.hpp"
#include <unordered_map>


template<typename EdgeList, typename AdjacentList>
class UndirectedGraph {
private:
    string name = "";
    int n_edges = 0;
    int n_vertices = 0;
    double density = .0;

    // Defining data structures to store the results
    unordered_map<int, Duration> elp_count_adj;
    unordered_map<int, Duration> elp_adj;
    unordered_map<int, size_t> tri_count_adj;


public:
    EdgeList edge_list;

    void printProprieties();
    void TriangleCounter(AdjacentList adjacent_list, const int num_threads);
    void GetResultByThread(const int thread);
    void WriteResultsCsv(string results_path);
    AdjacentList GetAdjacentList(const int num_threads);

    UndirectedGraph() { }

    // Constructor that given the path of a edges list it read it and store in the respective data structure
    UndirectedGraph(filesystem::directory_entry entry) {
        if (entry.is_regular_file()) {

            vector<string> row;
            string line, word;
            int n_row = 0;

            this->name = entry.path().generic_string();
            this->name.erase(this->name.begin(), this->name.begin() + 11);

            cout << "Reading file: " << this->name << " ...";

            ifstream file(entry.path(), ios::in);


            if (file.is_open()) {
                while (getline(file, line)) {
                    int first, second;

                    row.clear();

                    stringstream str(line);

                    while (getline(str, word, ','))
                        row.push_back(word);

                    if (n_row == 0) {
                        this->n_vertices = stoi(row[0]);
                    }
                    else {
                        first = stoi(row[0]);
                        second = stoi(row[1]);

                        this->edge_list.push_back(make_pair(first, second));
                    }

                    n_row += 1;

                }
            }
            file.close();

            this->n_edges = n_row - 1;
            this->density = static_cast<double>((2 * this->n_edges)) / (this->n_vertices * (this->n_vertices - 1));

            cout << "  DONE\n";
        }
    }
};


// Function to print useful graph proprieties
template<typename EdgeList, typename AdjacentList>
inline void UndirectedGraph<EdgeList, AdjacentList>::printProprieties()
{
    cout << "Name: " << this->name << " - Number of Edges: " << this->n_edges << " - Number of vertices: " << this->n_vertices << endl << endl;
}


// Function that implement the execution of counting the triangle in the actual UndirectedGraph object
template<typename EdgeList, typename AdjacentList>
inline void UndirectedGraph<EdgeList, AdjacentList>::TriangleCounter(AdjacentList adjacent_list, const int num_threads)
{
    size_t sum = 0;

    auto start = now();

#pragma omp parallel for schedule(dynamic) if(num_threads>1) num_threads(num_threads) reduction(+:sum)
    for (int i = 0; i < this->n_edges; ++i)
        sum += intersectionLength(adjacent_list[this->edge_list[i].first], adjacent_list[this->edge_list[i].second]);

    auto end = now();

    if (sum > 3) sum /= 3;

    this->elp_count_adj[num_threads - 1] = (end - start);
    this->tri_count_adj[num_threads - 1] = sum;

}



// Function to sprint in std output the result of a given execution with a given number of threads
template<typename EdgeList, typename AdjacentList>
inline void UndirectedGraph<EdgeList, AdjacentList>::GetResultByThread(const int thread)
{
    cout << "--- DONE ---\n" << fixed << "Triangle count: " << this->tri_count_adj[thread] << " - Execution time : " << this->elp_count_adj[thread].count() << "ms - Speed Up: " << this->elp_count_adj[0].count() / this->elp_count_adj[thread].count() <<
        " - Adjacent List Generation Time: " << this->elp_adj[thread].count() << "ms - Speed Up: " << this->elp_adj[0].count() / this->elp_adj[thread].count() << "\n\n";
}



// Function to write the results on the .csv file specified via its string path
template<typename EdgeList, typename AdjacentList>
inline void UndirectedGraph<EdgeList, AdjacentList>::WriteResultsCsv(string results_path)
{

    ofstream stream;
    stream.open(results_path, std::ios::out | std::ios::app);
    for (int i = 0; i < this->elp_count_adj.size(); ++i) {
                    //"name,             n_edges,                  n_vertices,            density,                threads
        stream << this->name << "," << this->n_edges << "," << this->n_vertices << "," << this->density << "," << (i + 1) << "," <<
            //    n_triangles,                 elapsed_triangle_count,                                   speed_up_traingle_count
            this->tri_count_adj[i] << "," << this->elp_count_adj[i].count() << "," << this->elp_count_adj[0].count() / this->elp_count_adj[i].count() << "," <<
            //elapsed_adjacent_list,                         speed_up_adjacent_list\n";
            this->elp_adj[i].count() << "," << this->elp_adj[0].count() / this->elp_adj[i].count() << "\n";
    }

    stream.close();
}



// Function that given the number of threads run in parallel the creation of the adjacent list for the actual UndirectedGraph object
template<typename EdgeList, typename AdjacentList>
inline AdjacentList UndirectedGraph<EdgeList, AdjacentList>::GetAdjacentList(const int num_threads)
{

    // Initialization of useful variables
    AdjacentList adjacent_list(this->n_vertices);
    vector<omp_lock_t> locks(this->n_vertices);
    int v1 = 0, v2 = 0;


    auto start = now();

    if (num_threads > 1)
#pragma omp parallel for schedule(dynamic) num_threads(num_threads)
        for (int i = 0; i < locks.size(); ++i)
            omp_init_lock(&locks[i]); // Initialize all the locks, each corresponding for as single adjacent list


    // Read the edge_list and populate the adjacent list
#pragma omp parallel for schedule(dynamic) if(num_threads > 1) num_threads(num_threads) shared(locks, adjacent_list)
    for (int i = 0; i < this->n_edges; ++i) {

        int v1 = this->edge_list[i].first;
        int v2 = this->edge_list[i].second;

        // In case the number of threads is greater then 1 we lock the corresponding adjacent list in position v1
        if (num_threads > 1) omp_set_lock(&locks[v1]);
        auto it1 = lower_bound(adjacent_list[v1].begin(), adjacent_list[v1].end(), v2);
        adjacent_list[v1].insert(it1, v2);
        if (num_threads > 1) omp_unset_lock(&locks[v1]); // Finishing the insertion we release the lock for the corresponding adjacent list

        // In case the number of threads is greater then 1 we lock the corresponding adjacent list in position v2
        if (num_threads > 1) omp_set_lock(&locks[v2]);
        auto it2 = lower_bound(adjacent_list[v2].begin(), adjacent_list[v2].end(), v1);
        adjacent_list[v2].insert(it2, v1);
        if (num_threads > 1) omp_unset_lock(&locks[v2]); // Finishing the insertion we release the lock for the corresponding adjacent list

    }


    if (num_threads > 1)
#pragma omp parallel for schedule(dynamic) num_threads(num_threads)
        for (int i = 0; i < locks.size(); ++i)
            omp_destroy_lock(&locks[i]); // Destroy all the locks, each corresponding for as single adjacent list


    auto end = now();

    this->elp_adj[num_threads - 1] = (end - start);

    return adjacent_list;
}



// Function that return the vector of UndirectedGraph given the string path
vector<UndirectedGraph<EdgeList, AdjacentList>> ReadFromDirectory(string path) {
    vector<UndirectedGraph<EdgeList, AdjacentList>> graphs_list;

    for (const auto& entry : filesystem::directory_iterator(path))
        graphs_list.push_back(UndirectedGraph<EdgeList, AdjacentList>(entry));

    return graphs_list;
}


#endif
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
//#include <set>


using namespace std;

//using Duration = std::chrono::high_resolution_clock::duration;
static auto now = std::chrono::high_resolution_clock::now;
using Duration = std::chrono::duration<double, milli>;
static std::mt19937 prng{ std::random_device{}() };
using Distribution = std::uniform_int_distribution<int>;

using AdjacentList = unordered_map<int, vector<int>>;
using EdgeList = vector<pair<int, int>>;


// TO FIX
/*struct fmt {
    Duration const& _d;

    friend std::ostream& operator<<(std::ostream& os, fmt f) {
        if (f._d >= 1min) return os << (f._d / 1min) << "min " << (f._d % 1min) / 1s << "s";
        else if (f._d >= 1s) return os << (f._d / 1.0s) << "s";
        else if (f._d >= 1ms) return os << (f._d / 1.0ms) << "ms";
        else return os << (f._d / 1.0us) << "us";
    }
};*/

size_t intersectionLength(vector<int>* v1, vector<int>* v2)
{   
    //sort((*v1).begin(), (*v1).end());
    //sort((*v2).begin(), (*v2).end());
    vector<int> intersection;
    set_intersection((*v1).begin(), (*v1).end(), (*v2).begin(), (*v2).end(), back_inserter(intersection));
    return intersection.size();
}


size_t TriangleCount(EdgeList *edge_list, AdjacentList *adjacent_list, int start, int skip) { //vector<bool>* sorted
    size_t sum = 0;
    for (int i = start; i < (*edge_list).size(); i += skip) {

        // add pointer of pointer 

        /*if ((*sorted)[(*edge_list)[i].first]) {
            sort((*adjacent_list)[(*edge_list)[i].first].begin(), (*adjacent_list)[(*edge_list)[i].first].end());
            (*sorted)[(*edge_list)[i].first] = true;
        }
        if ((*sorted)[(*edge_list)[i].second]) {
            sort((*adjacent_list)[(*edge_list)[i].second].begin(), (*adjacent_list)[(*edge_list)[i].second].end());
            (*sorted)[(*edge_list)[i].second] = true;
        }*/

        sum += intersectionLength(&(*adjacent_list)[(*edge_list)[i].first], &(*adjacent_list)[(*edge_list)[i].second]);
    }
    return sum;
}

#endif
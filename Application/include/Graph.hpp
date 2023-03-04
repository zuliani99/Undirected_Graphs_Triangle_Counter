#ifndef _Graph_H
#define _Graph_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <random>
#include <chrono>


using namespace std::chrono_literals;
static auto now = std::chrono::high_resolution_clock::now;
using Duration = std::chrono::high_resolution_clock::duration;
static std::mt19937 prng{ std::random_device{}() };

template<typename T>
using Distribution = std::uniform_int_distribution<T>;


using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, boost::no_property>;
using E = Graph::edge_descriptor;
using V = Graph::vertex_descriptor;
using edge_iterator = boost::graph_traits<Graph>::edge_iterator;


// Struct used to manage the output of both algorithms execution time 
struct fmt
{
    Duration const& _d;

    friend std::ostream& operator<<(std::ostream& os, fmt f)
    {
        if (f._d >= 1min) return os << (f._d / 1min) << "min " << (f._d % 1min) / 1s << "s";
        else if (f._d >= 1s) return os << (f._d / 1.0s) << "s";
        else if (f._d >= 1ms) return os << (f._d / 1.0ms) << "ms";
        else return os << (f._d / 1.0us) << "us";
    }
};

#endif
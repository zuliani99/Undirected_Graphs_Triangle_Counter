#include "../include/UndirectedGraph.h"

// Function to generate a random bipartite graph, complete or incomplete, depending on the user input
Graph generateData(int N, int M) {
	Graph g;

	Distribution<int>dist_edge(N-1, N*(N-1)/M);
	Distribution<int>dist_vertex(0, N);

	for(int i = 0; i < dist_edge(prng); i++) {
		int v1 = dist_vertex(prng);
		int v2 = 0;
		do{ int v2 = dist_vertex(prng); } while(v1 == v2 && boost::edge(v1,v2,g).second);

		boost::add_edge(v1, v2, g);
		
	}

	return g;

}

void printGraph(Graph& g) {
    boost::dynamic_properties dp;
    dp.property("node_id", get(boost::vertex_index, g));
    dp.property("weight", get(boost::edge_weight, g));
    boost::write_graphviz_dp(std::cout, g, dp);
}
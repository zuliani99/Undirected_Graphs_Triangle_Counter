#ifndef _SeqTC_H
#define _SeqTC_H

#include "Graph.hpp"

int sequentialCount(cost Graph *g) {
	int count = 0
	std::pair<edge_iterator, edge_iterator> ei = boost::edges(g);
	for(edge_iterator edge_iter = ei.first; edge_iter != ei.second; ++edge_iter) {
		auto iter_first = boost::adjacent_vertices(edge_iter.first, g);
		auto iter_second = boost::adjacent_vertices(edge_iter.first, g);

		// non è ottimizzata
		for (auto v_iter_1 : make_iterator_range(iter_first))
			for (auto v_iter_2 : make_iterator_range(iter_first))
				if (v_iter_1 == v_iter_2)
					count += 1
	}

	return int(count /= 3)
}



#endif


#include "../include/Utils.hpp"
#include "../include/UndirectedGraph.hpp"

void RunTriangleCounter(vector<UndirectedGraph<EdgeList, AdjacentList>> graphs_vector, string results_path) {
	for (auto& graph : graphs_vector) {
		cout << "\n";
        graph.printProprieties();

        cout << "Number of available cores: " << thread::hardware_concurrency() << "\n\n";

        cout << "SEQUENTIAL EXECUTION ...\n";
        graph.SequentialTriangleCounter();
        graph.GetResultByThread(1);

        for (int threads = 2; threads <= static_cast<int>(thread::hardware_concurrency()) * 2; threads++) {
            cout << "PARALLEL EXECUTION WITH " << threads << " THREADS\n";
            graph.ParallelTriangleCounter(threads);
            graph.GetResultByThread(threads);
        }

        graph.WriteResultsCsv(results_path);

        cout << "\n\n";
	}
}


int main() {
	srand(time(NULL));
    string standford_datasets_path = "../datasets/standford";
	string random_datasets_path = "../datasets/random_graphs";
    string results_path;

	bool newer_graph = false;
    
    vector<UndirectedGraph<EdgeList, AdjacentList>> random_graphs;

	cout << "Do you want to generate newer random graphs? (0/1)" << endl;
	cin >> newer_graph;

	if (newer_graph != 0 && newer_graph != 1) 
		throw std::invalid_argument("Please insert correct input");
	
	if (newer_graph) {
		const int initial_n_vertices = 10;
    	const int finaal_n_vertices = 1000000;
    	const int n_steps = 5;

		for(int i = 0; i < n_steps; ++i) {

			Distribution vertex_dist(initial_n_vertices + (i * 10), (initial_n_vertices * 10 + (i * 10) - 1));
			int random_n_vertices = vertex_dist(prng);
			cout << random_n_vertices << endl;

			Distribution sparse_dist((random_n_vertices - 1), random_n_vertices * (random_n_vertices - 1) / 4);
			Distribution dense_dist((random_n_vertices * (random_n_vertices - 1) / 4) + 1, random_n_vertices * (random_n_vertices - 1) / 2);

			//random_graphs->push_back(UndirectedGraph<EdgeList, AdjacentList>(random_n_vertices, sparse_dist(prng)));
			//random_graphs->push_back(UndirectedGraph<EdgeList, AdjacentList>(random_n_vertices, dense_dist(prng)));
			int sparse = sparse_dist(prng);
			int dense = dense_dist(prng);
			cout << sparse << endl;
			cout << dense << endl << endl;
			GenerateAndWriteRandomGraph(random_n_vertices, sparse);
			GenerateAndWriteRandomGraph(random_n_vertices, dense);

		}
	}

	random_graphs = ReadFromDirectory(random_datasets_path);
    vector<UndirectedGraph<EdgeList, AdjacentList>> standford_graphs = ReadFromDirectory(standford_datasets_path);


	results_path = ReturnResultPath();

    // initialization of the results.csv

    //time_t result = time(NULL);
    //char timestamp[26];
    //ctime(timestamp, sizeof(timestamp), &result);

	/*time_t curtime;
    time(&curtime);
    string ts = string(ctime(&curtime));
    ts.erase(remove(ts.begin(), ts.end(), '\n'), ts.cend());

    replace(ts.begin(), ts.end(), ' ', '_');
    replace(ts.begin(), ts.end(), ':', '.');
    
    results_path << "../results/results_" << ts << ".csv";*/


    fstream stream;
    stream.open(results_path, std::ios::out | std::ios::app);
    stream << "name,n_edges,n_vertices,density,threads,n_triangles,elapsed,speed_up\n";
    stream.close();

	RunTriangleCounter(random_graphs, results_path);
	RunTriangleCounter(standford_graphs, results_path);


    return 0;
}


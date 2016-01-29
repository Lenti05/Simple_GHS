//
//  main.cpp
//  Synch_GHS
//
//  Created by Pietro Lenti on 19/10/2015.
//  Copyright © 2015 Pietro Lenti. All rights reserved.
//
// This is a simplified version of the legendary algorithm GHS ([1]). It is basically the result of the application of a synchronizer ([2]) to the
// traditional algorithm. The algorithm (called Synch_GHS) has the same time complexity upper bound as GHS, namely O(n*log(n)), where n is the number of nodes
// of the network. The communication complexity is worse because of the synchronization messages used at every level: now it is O(|E|*log(n)), where |E| is the
// number of edges of the network.
//
// REFERENCES:
// [1] Gallager, R.G., Humblet, P.A. and Spira, P.M.: A distributed algorithm for minimum weight spanning trees. ACM Transaction on Programming Languages
// and Systems, 55: 66-77, 1983.
// [2] Boruch Awerbach. Complexity of nerwork synchronization. Journal of the ACM. 35(4): 845-875, October 1988.

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <fstream>



#include "split.hpp"
#include "Node.hpp"
#include "Output_edges.hpp"

using std::vector;
using std::string;

// INPUT: File containing the adjancency matrix of the network.
// OUTPUT:
// 1) MST.
// 2) Index of the root of MST.
// 3) Log files of each node.

// The main thread reads the file containg the adjacency matrix related to the network and starts the algorithm waking up all the nodes.
// On standard output is printed the MST, reporting all the edges with their label (branch or rejected) and the index of the leader (root node
// of the MST). Furthermore, each node writes its own log files containing all the phases of algorithm.
int main(int argc, const char * argv[]) {
    std::string dir_path_inputs = ("../Test/Input/");
    std::string dir_path_log_files = ("../Test/Log_files/adj_matr6/");
    std::string file_input = (dir_path_inputs + "adj_matr6.txt");
    std::vector<Node> vector_nodes;
    File_ptr fp(file_input, "r");
    char n_nodes[50];
    fp.read(n_nodes);
    int n = to<int>(n_nodes);
    vector<Output_edges> output_vector(n);
    vector<Synch_queue> queues_vector(n);
    vector<File_ptr*> log_writers(n);
    for(int i = 0; i < n; i++)
        queues_vector[i].initialize(i);
    for(int i = 0; i < n; i++)
        vector_nodes.push_back(Node{i, &queues_vector[i], &output_vector[i],  dir_path_log_files  });
    for(int i=0; i< n; i++) {
        char line[300];
        fp.read(line);
        vector<string> weights_vector = split(line);
        vector<Edge> edges_vector;
        for(int j = 0; j<n; j++) {
            if(to<int>(weights_vector[j]) != 0) {
                Edge edge{j, to<int>(weights_vector[j]), vector_nodes[j].get_queue_pt()};
                edges_vector.push_back(edge);
            }
        }
        vector_nodes[i].initialize_edges(edges_vector);
    }
    vector<std::thread> t_nodes;
    for(int i=0; i< n; i++)
    {
        vector_nodes[i].wakeup();
        t_nodes.push_back(std::thread{std::move(vector_nodes[i])});
    }
    for(int i=0; i< n; i++)
    {
        t_nodes[i].join();
        output_vector[i].print();
    }
    return 0;
}




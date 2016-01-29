//
//  Output_edges.hpp
//  Synch_GHS
//
//  Created by Pietro Lenti on 12/01/2016.
//  Copyright © 2016 Pietro Lenti. All rights reserved.
//

#ifndef Output_edges_hpp
#define Output_edges_hpp

#include <stdio.h>
#include <iostream>
#include "to.hpp"
#include "Edge.hpp"
#include <map>


class Output_edges {
public:
    Output_edges()
    : num_adj_nodes(0), index(-1) {}
    void add_edge(int id, Status final_status) { edges.insert(std::pair<int, Status>(id,final_status));}
    void set_index(int id) {index = id;}
    void print()
    {
        for(auto edge:edges)
        std::cout << "Edge " + to<>(index) + "-" + to<>(edge.first) + "): " + get_status_str(edge.second) << "\n";
        }
    
    std::string get_status_str(Status status){
        if(status == Status::branch)
            return  ("branch");
        else
            return  ("rejected");
    }
    
private:
    std::map<int, Status> edges;
    int num_adj_nodes;
    int index;
};

#endif /* Output_edges_hpp */

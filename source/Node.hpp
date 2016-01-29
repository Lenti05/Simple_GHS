//
//  Node.hpp
//  Synch_GHS
//
//  Created by Pietro Lenti on 19/10/2015.
//  Copyright © 2015 Pietro Lenti. All rights reserved.
//

#ifndef Node_hpp
#define Node_hpp

#include <stdio.h>

#include<map>
#include<string>
#include<vector>

#include"Edge.hpp"
#include"File_ptr.hpp"
#include"to.hpp"
#include"Output_edges.hpp"

using namespace std;

enum class State  {sleeping, awake};


class Node {
public:
    //constructor
    Node(const int id, Synch_queue* q, Output_edges* output,  std::string log_files_path ):index(id),
      fp(log_files_path + "node_log" + to<>(id) + ".txt", "w"), message_queue(q), output_edges(output), interrupt(false)
    {}
    //copy constructor
    Node(const Node&_node) = delete;
    //move constructor
    Node(Node&& _node): index{_node.index}, fp{std::move(_node.fp)}, message_queue{_node.message_queue}, interrupt{_node.interrupt},  adj_edges{_node.adj_edges}, level{_node.level}, frag_ID{_node.frag_ID}, output_edges{_node.output_edges},
        state{_node.state}, counter_branches{_node.counter_branches}, counter_test{_node.counter_test}, counter_rejected{_node.counter_rejected},
        counter_initiate{_node.counter_initiate}
    {
        _node.message_queue = nullptr;
        _node.adj_edges.erase(_node.adj_edges.begin(), _node.adj_edges.end());
    }
    
    void wakeup();
    void initialize_edges(vector<Edge>&);
    void end_of_test();
    void changeroot();
    // transaction can be either a deposit or a withdrawal
    void transaction(double amount, int receiver_index);
    void answer_to_connect(Message&);
    void answer_to_initiate(Message&);
    void answer_to_test(Message& );
    void answer_to_report(Message&);
    void answer_to_changeroot(Message&);
    void answer_to_ack(Message&);
    void answer_to_finished(Message&);
    void answer_to_reject(Message&);
    const int get_index();
    Synch_queue* get_queue_pt();
    void process_message(Message&);
    void clear()
    {
        for(auto link:adj_edges)
            link.second.set_status(Status::basic);
    }
    void unlock_queue(){message_queue->unlock();}
    void operator()();
    
private:
    std::map<int, Edge> adj_edges;
    const int index;
    int level;
    int frag_ID;
    State state;
    int best_edge_index;
    int best_weight;
    int parent_index;
    int counter_branches;
    int counter_test;
    int counter_rejected;
    int counter_initiate;
    int counter_finishing;
    File_ptr fp;
    Output_edges* output_edges;
    Synch_queue* message_queue;
    bool interrupt;
};
#endif /* Node_hpp */

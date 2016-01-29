//
//  Edge.hpp
//  Synch_GHS
//
//  Created by Pietro Lenti on 19/10/2015.
//  Copyright © 2015 Pietro Lenti. All rights reserved.
//
#ifndef Edge_hpp
#define Edge_hpp

#include <stdio.h>
#include <string>

#include "Synch_queue.hpp"


enum class Status {basic, branch, rejected};

class Edge {
public:
    //Default constructor
    Edge() {}
    //Constructor
    Edge( int ID, int w, Synch_queue* q):index(ID), status(Status::basic), weight(w), mq(q)
    {}
    //Copy constructor
    Edge(const Edge& edge): index{edge.index}, status{edge.status}, weight{edge.weight}, mq{edge.mq} {}
    //Move constructor
    Edge(Edge&& edge): index{edge.index}, status{edge.status}, weight{edge.weight}, mq{edge.mq}
    {
        edge.mq = nullptr;
    }
    const int get_weight() { return weight; }
    const int get_index() { return index; }
    
    std::string get_status_str(){
        if(status == Status::basic)
            return ("basic");
        if(status == Status::branch)
            return  ("branch");
        else
            return  ("rejected");
    }
    Status get_status() { return status; }
    void set_status(Status status) { this->status = status; }
    void send_message(Message& msg) {mq->put(msg);}
    void send_highprior_message(Message& msg) {mq->put_first(msg);}
    
private:
    int index;
    Status status;
    int weight;
    Synch_queue* mq;
};










#endif /* Edge_hpp */

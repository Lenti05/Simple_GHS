//
//  Node.cpp
//  Synch_GHS
//
//  Created by Pietro Lenti on 19/10/2015.
//  Copyright © 2015 Pietro Lenti. All rights reserved.
//


#include "Node.hpp"

#define  INFINITY 10000

using std::vector;
using std::string;


const int Node::get_index(){return index;}
Synch_queue* Node::get_queue_pt(){return message_queue;}

void Node::initialize_edges(vector<Edge>& neighbors){
    for(auto neighbor:neighbors)
        adj_edges.insert(std::move(std::pair<int,Edge>(neighbor.get_index(),neighbor)));
}



void Node::process_message(Message& msg)
{
    
    switch (msg.get_type()) {
        case Type::connect:
            Node::answer_to_connect(msg);
            break;
            
        case Type::initiate:
            Node::answer_to_initiate(msg);
            break;
            
        case Type::test:
            Node::answer_to_test(msg);
            break;
            
        case Type::report:
            Node::answer_to_report(msg);
            break;
            
        case Type::reject:
            Node::answer_to_reject(msg);
            break;
            
        case Type::changeroot:
            Node::answer_to_changeroot(msg);
            break;
            
        case Type::ack:
            Node::answer_to_ack(msg);
            break;
            
        case Type::finished:
            Node::answer_to_finished(msg);
            break;
            
        default:
            cout << "There is something wrong!\n";
            break;
    }
}

//When a node wakes up, it detects its minimum outgoing weight edge(MWOE) and sends a CONNECT message on that edge.
void Node::wakeup() {
    
    fp.write("Node" + to<>(index) + " Doing process WAKEUP");
    int min_wt = INFINITY;
    int min_index = -1;
    for (auto link:adj_edges)
    {
       if(link.second.get_weight() < min_wt)
       {
           min_wt = link.second.get_weight();
           min_index = link.first;
       }
    }
    fp.write("Edge(" + to<>(index) + "-" + to<>(min_index) + ") set to BRANCH");
    adj_edges[min_index].set_status(Status::branch);
    fp.write("Fragment-level: 0");
    level = 0;
    fp.write("Node-state: sleeping -> alive");
    state = State::awake;
    counter_branches = 1;
    counter_test = 0;
    counter_rejected = 0;
    counter_initiate = 0;
    frag_ID = index;
    fp.write("Sending Connect on (" + to<>(index) + "-" + to<>(min_index) + ")");
    std::vector<int> msg_parameters;
    msg_parameters.push_back(0);
    Message msg(Type::connect, index, msg_parameters);
    adj_edges[min_index].send_message(msg);
}


// When a cluster attempts to combine wiht another cluster, it sends a CONNECT message across the MWOE. If the receiver has the same MWOE, a merge
// operation occurs. A cluster resulting from a merge operation has the same level of the merging cluster increased by one. If the sender has a lower
// level than the receiver, an absorb operation occurs: the sender's cluster is absorbed into the receiver's and the resulting cluster has the same level
// of the receiver. Finally, if the sender has the same level of the receiver, the message is put at the end of the message queue, which it means that
// when the receiver eventually increases its level (after a merge or absorb process with another cluster) an absorb operation will occur.

void Node::answer_to_connect(Message& msg){
    int sender_index = msg.get_sender_ID();
    int sender_level = msg.get_parameters()[0];
    fp.write("Recv: Connect from host" + to<>(sender_index) + " on (" + to<>(sender_index) + "-" +
    to<>(index) + ")");
    //to<>(index) + ")");
    if(state == State::sleeping)
        wakeup();
    //Connection request from a node across MWOE.
    if(adj_edges[sender_index].get_status() == Status::branch){
        int leader_index = std::min(index, sender_index);
        fp.write("Sending Initiate " + to<>(level + 1) + " " + to<>(leader_index) + " on " + to<>(index) + "-" + to<>(sender_index));
        vector<int> msg_parameters;
        msg_parameters.push_back(level + 1);
        msg_parameters.push_back(leader_index);
        Message msg(Type::initiate, index, msg_parameters);
        adj_edges[sender_index].send_message(msg);
    }
    //Connection request from a node with the same level on an edge that is not the MWOE
    else if(sender_level == level  ){
         fp.write("place received message at the end of the queue");
         message_queue->put(msg);
    }
    //Connection request from a node with a lower level
    else {
         adj_edges[sender_index].set_status(Status::branch);
         counter_branches++;
         fp.write("Sending Initiate " + to<>(level) + " " + to<>(frag_ID) + " on " + to<>(index) + "-" + to<>(sender_index));
         vector<int> msg_parameters;
         msg_parameters.push_back(level);
         msg_parameters.push_back(frag_ID);
         Message msg(Type::initiate, index, msg_parameters);
         adj_edges[sender_index].send_message(msg);
     }
}

// When two clusters merge to form a new cluster, a leader node for the resulting cluster is elected.
// The leader node is the one with the lowest ID among the two sharing their MWOE. When the leader node is elected, the first task to be done is to update the
// cluster ID and level of all the nodes belonging to the minimum spanning tree sub tree (MST sub tree) having the leader as root node. In order to do that,
// an INITIATE message is broadcast throughout the cluster, starting at the leader, along at the cluster's spanning tree. When the nodes at the boundaries of
// the tree receive the INITIATE message they convergecast an ACK message back to the leader.

void Node::answer_to_initiate(Message& msg){
    int sender_index = msg.get_sender_ID();
    fp.write("Recv: Initiate from host" + to<>(sender_index) + " on (" + to<>(sender_index) + "-" +
    to<>(index) + ")");
    //to<>(index) + ")");
    level = msg.get_parameters()[0];
    best_weight = INFINITY;
    frag_ID = msg.get_parameters()[1];
    parent_index = sender_index;
    best_edge_index = -1;
    //Counter_initiate is different for the leader because he waits for Ack messages from all his neighbors on branch edges
    if(frag_ID == index)
        counter_initiate = counter_branches;
    //Whereas non-leader nodes waits for ACK messages from all the neighbors on branch edges except for the parent node.
    else
        counter_initiate = counter_branches - 1;
    if(counter_initiate == 0){
        fp.write("Sending ack on " + to<>(index) + "-" + to<>(parent_index));
        vector<int> msg_parameters;
        Message msg(Type::ack, index, msg_parameters);
        adj_edges[parent_index].send_message(msg);
    }
    else{
        for(auto link:adj_edges){
            if(link.second.get_status()==Status::branch && link.second.get_index() != parent_index){
              fp.write("Sending Initiate " + to<>(level) + " " + to<>(frag_ID) + " on " + to<>(index) + "-" + to<>(link.second.get_index()));
              vector<int> msg_parameters;
              msg_parameters.push_back(level);
              msg_parameters.push_back(frag_ID);
              Message msg(Type::initiate, index, msg_parameters);
              adj_edges[link.second.get_index()].send_message(msg);
            }
        }
       }
}

// The "initiate" phase terminates when the leader receives ACK messages from all the neighbor nodes across branch edges. The leader, then, can start the
// "test" phase in which the cluster firstly absorbs all the other clusters having lower level connected to it (namely having MWOW among the outgoing edges of the
// cluster in exam) and secondly starts the research for a new MWOE begins. This phase is triggered by the leader that sends TEST messages to all its neighbors
// whose edges have not been put to rejected.

void Node::answer_to_ack(Message& msg ){
    int sender_index = msg.get_sender_ID();
    fp.write("Recv: Ack from host" + to<>(sender_index) + " on (" + to<>(sender_index) + "-" +
    to<>(index) + ")");
    //to<>(index) + ")");
    counter_initiate--;
    if(counter_initiate == 0) {
        if(frag_ID != index) {
            fp.write("Sending Ack on " + to<>(index) + "-" + to<>(parent_index));
            vector<int> msg_parameters;
            Message msg(Type::ack, index, msg_parameters);
            adj_edges[parent_index].send_message(msg);
        }
        //Once the leader has received all the ACK messages on all the branches connected to him, the "test" phase can start.
        else{
            fp.write("Cluster" + to<>(frag_ID) + "is formed with level " + to<>(level));
            counter_test = adj_edges.size() - counter_rejected;
            for(auto link: adj_edges)
                if(link.second.get_status() != Status::rejected){
                  fp.write("Sending Test " + to<>(level) + " " + to<>(frag_ID) + " on " + to<>(index) + "-" + to<>(link.second.get_index()));
                  vector<int> msg_parameters;
                  msg_parameters.push_back(level);
                  msg_parameters.push_back(frag_ID);
                  Message msg(Type::test, index, msg_parameters);
                  adj_edges[link.second.get_index()].send_message(msg);
                }
        }
    }
}

// The research fo the MWOE for a given cluster, for purpose of synchronization, cannot start until all the neighbor clusters have a level equal or
// higher to the one of the cluster in exam. In order to attempt this task, four possible scenarios are  possible when a node receive a TEST message:
// 1) The receiver has the same cluster ID of the sender and it is on a basic edge.
// 2) The receiver has the same cluster ID of the sender and it is on a branch edge.
// 3) The receiver has a different cluster ID but an higher or equal level.
// 4) The receiver has a different cluster ID but a lower level.
// In the first scenario the edge is rejected by the receiver and a REJECT message is sent back to the sender.
// In the second scenario the receiver sends TEST messages to all its neighbors, except for the parent node, whose edges have not been rejected.
// In the third scenario the receiver sends a REPORT message containing the weight of the edge connecting it to the sender.
// In the fourth scenario the message is placed on the end of the message queue. For this latter case two situations eventually arise.
// First, the receiver is eventually merged or absorbed in a cluster with the same level or higher than the sender; in this situation we go back
// to the case number three.
// Second, the receiver is eventually absorbed in the same cluster of the receiver; in this situation we go back to cases number 1 or 2 accordingly.

void Node::answer_to_test(Message& msg) {
    int sender_index = msg.get_sender_ID();
    int sender_level = msg.get_parameters()[0];
    int sender_frag_ID = msg.get_parameters()[1];
    fp.write("Recv: Test from host" + to<>(sender_index) + " on (" + to<>(sender_index) + "-" +
             to<>(index) + " )");
      //       to<>(index) + " )");
    if(state == State::sleeping)
        wakeup();
    if(frag_ID != sender_frag_ID){
        // Scenario number 3.
        if(level == sender_level || level > sender_level){
            fp.write("Sending Report " + to<>(adj_edges[sender_index].get_weight()) + " on " + to<>(index) + "-" + to<>(sender_index));
                  vector<int> msg_parameters;
                  msg_parameters.push_back(adj_edges[sender_index].get_weight());
                  Message msg(Type::report, index, msg_parameters);
                  adj_edges[sender_index].send_message(msg);
        }
        // Scenario number 4.
        else{
            fp.write("Place the received message a the end of the queue");
            message_queue->put(msg);
        }
    }
    else{
        // Scenario number 1.
        if(adj_edges[sender_index].get_status() == Status::basic){
            adj_edges[sender_index].set_status(Status::rejected);
            counter_rejected++;
            fp.write("Sending Reject on " + to<>(index) + "-" + to<>(sender_index));
            vector<int> msg_parameters;
            Message msg(Type::reject, index, msg_parameters);
            adj_edges[sender_index].send_message(msg);
        }
        // Scenario number 2.
        else{
            counter_test = adj_edges.size() - counter_rejected -1;
            if(counter_test == 0)
                end_of_test();
            else {
                for(auto link: adj_edges) {
                    if(link.second.get_index() != sender_index && link.second.get_status() != Status::rejected){
                      fp.write("Sending Test " + to<>(level) + " " + to<>(frag_ID) + " on " + to<>(index) + "-" + to<>(link.second.get_index()));
                      vector<int> msg_parameters;
                      msg_parameters.push_back(level);
                      msg_parameters.push_back(frag_ID);
                      Message msg(Type::test, index, msg_parameters);
                      adj_edges[link.second.get_index()].send_message(msg);
                    }
                 }
             }
          }

         }
}

// The answers to TEST messages are (immediately or eventually of two different kind: REPORT messages and REJECT messages.
// The REPORT messages convergecast information about minimum weight edges back to the leader.

void Node::answer_to_report(Message& msg) {
    int sender_index = msg.get_sender_ID();
    int sender_weight = msg.get_parameters()[0];
    fp.write("Recv: Report " + to<>(sender_weight) + " from host" + to<>(sender_index) + " on (" + to<>(sender_index) + "-" + to<>(index));
    counter_test--;
    if(sender_weight < best_weight) {
        best_weight = sender_weight;
        best_edge_index = sender_index;
    }
    if(counter_test == 0)
        end_of_test();
}

// Whereas REJECT messages tell the testing node which of its edges must be rejected because leading to a node in the same component.
void Node::answer_to_reject(Message& msg) {
    int sender_index = msg.get_sender_ID();
    fp.write("Recv: Reject from host" + to<>(sender_index) + " on (" + to<>(sender_index) + "-" + to<>(index));
    counter_test--;
    if(adj_edges[sender_index].get_status() == Status::basic) {
    adj_edges[sender_index].set_status(Status::rejected);
    counter_rejected++;
    }
    if(counter_test == 0)
        end_of_test();
}

// When a node receives all the answers to its TEST probes, it convergecasts the minimum weight of all the weights its responding nodes have
// convergecast to it.
// If the node in exam is the leader, it can either starts the "changeroot" phase or ends the algorithm if its best weight is infinity (which it means
// that no MWOW is found, situation that occurs only when one cluster eventually remains).

void Node::end_of_test(){
    fp.write("Doing procedure end of test");
    //if this node is not the leader
    if(frag_ID != index) {
            fp.write("Sending Report " + to<>(best_weight) + " on " + to<>(index) + "-" + to<>(parent_index));
            vector<int> msg_parameters;
            msg_parameters.push_back(best_weight);
            Message msg(Type::report, index, msg_parameters);
            adj_edges[parent_index].send_message(msg);
    }
    //if this node is the leader
    else {
        if (best_weight == INFINITY) {
            fp.write("Node " + to<>(index) + " is leader");
            std::cout << "The leader of the MST is the node " << index << "\n";
            counter_finishing = counter_branches;
            output_edges->set_index(index);
            for(auto link: adj_edges){
                fp.write("Edge(" + to<>(index) + "-" + to<>(link.second.get_index()) + "): " + link.second.get_status_str());
                output_edges->add_edge(link.second.get_index(), link.second.get_status());
                if(link.second.get_status() == Status::branch) {
                      fp.write("Sending finished on " + to<>(index) + "-" + to<>(link.second.get_index()));
                      vector<int> msg_parameters;
                      Message msg(Type::finished, index, msg_parameters);
                      adj_edges[link.second.get_index()].send_message(msg);
                }
            }
            fp.write("Terminating process: host " + to<>(index));
            interrupt = true;
        }
        else
            changeroot();
    }
}

// The "changeroot" phase starts when the leader has received all the REPORT and REJECT messages by all the tested nodes. A CHANGEROOT message is then sent
// from the leader towards the component process that is adjacent to the component's MWOE.


void Node::answer_to_changeroot(Message& msg)
{
    int sender_index = msg.get_sender_ID();
    fp.write("Recv: Changeroot from host" + to<>(sender_index) + " on (" + to<>(sender_index) + "-" + to<>(index));
    changeroot();
}

// That node finally sends A CONNECT message across the MWOE.

void Node::changeroot()
{
    fp.write("Doing procedure changeroot");
    if(adj_edges[best_edge_index].get_status() == Status::branch){
        fp.write("Sending Changeroot on " + to<>(index) + "-" + to<>(best_edge_index));
        vector<int> msg_parameters;
        Message msg(Type::changeroot, index, msg_parameters);
        adj_edges[best_edge_index].send_message(msg);
    }
    else {
        adj_edges[best_edge_index].set_status(Status::branch);
        counter_branches++;
        fp.write("Sending Connect " + to<>(level) + " on (" + to<>(index) + "-" + to<>(best_edge_index) + ")");
        std::vector<int> msg_parameters;
        msg_parameters.push_back(level);
        Message msg(Type::connect, index, msg_parameters);
        adj_edges[best_edge_index].send_message(msg);
    }
}

// The "finish" phase is a simple broadcast procedure from the leader of the MST (which coincides with the leader of the last cluster to remain).

void Node::answer_to_finished(Message& msg)
{
    int sender_index = msg.get_sender_ID();
    fp.write("Recv: Finished from host " + to<>(sender_index) + "on " + to<>(sender_index) + "-" + to<>(index));
    output_edges->set_index(index);
    for(auto link: adj_edges){
        output_edges->add_edge(link.second.get_index(), link.second.get_status());
        fp.write("Edge(" + to<>(index) + "-" + to<>(link.second.get_index()) + "): " + link.second.get_status_str());
        if(link.second.get_status() == Status::branch && link.second.get_index() != parent_index){
        fp.write("Sending finished on " + to<>(index) + "-" + to<>(link.second.get_index()));
        vector<int> msg_parameters;
        Message msg(Type::finished, index, msg_parameters);
        adj_edges[link.second.get_index()].send_message(msg);
        }
    }
        fp.write("Terminating process: host " + to<>(index));
        interrupt = true;
}

// The operator() function of Node class just gets the message from the head of its own message queue.

void Node::operator()(){
    while(!interrupt)
    {
        Message msg;
        message_queue->get(msg);
        process_message(msg);
    }
}





    
    
    




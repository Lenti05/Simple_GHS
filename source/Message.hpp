//
//  Message.hpp
//  Synch_GHS
//
//  Created by Pietro Lenti on 19/10/2015.
//  Copyright © 2015 Pietro Lenti. All rights reserved.
//

#ifndef Message_hpp
#define Message_hpp

#include <stdio.h>
#include <string>
#include <vector>

enum class Type {connect, initiate, report, reject, test, changeroot, ack, finished};

class Message{
    
public:
    Message():sender_ID(-1) {}
    Message(Type msg_type, int index, std::vector<int> msg_parameters):type(msg_type)
    , sender_ID(index), parameters(msg_parameters.size())
    { std::copy(msg_parameters.begin(), msg_parameters.end(), parameters.begin());}

    Type get_type(){return type;}
    const int get_sender_ID(){return sender_ID;}
    std::vector<int> get_parameters(){return parameters;}
    
private:
    Type type;
    int sender_ID;
    std::vector<int> parameters;
};

#endif /* Message_hpp */

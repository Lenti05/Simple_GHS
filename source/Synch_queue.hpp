//
//  Synch_queue.hpp
//  Synch_GHS
//
//  Created by Pietro Lenti on 19/10/2015.
//  Copyright © 2015 Pietro Lenti. All rights reserved.
//

#ifndef Synch_queue_hpp
#define Synch_queue_hpp

#include <stdio.h>
#include <list>
#include <mutex>
#include <string>
#include <condition_variable>

#include "Message.hpp"




class Synch_queue {
public:
    Synch_queue(): index{0} {}
    Synch_queue(const int ID):index{ID}{}
    void initialize(int ID) {index = ID;}
    void put(const Message& msg);
    void put_first(const Message& msg);
    void get(Message& msg);
    void unlock(){mtx.unlock();}
private:
    int index;
    std::mutex mtx;
    std::condition_variable cond;
    std::list<Message> q;
};



#endif /* Synch_queue_hpp */

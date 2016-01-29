//
//  Synch_queue.cpp
//  Synch_GHS
//
//  Created by Pietro Lenti on 19/10/2015.
//  Copyright © 2015 Pietro Lenti. All rights reserved.
//
#include "Synch_queue.hpp"

#include<iostream>
#include<mutex>
#include<string>


void Synch_queue::put(const Message& msg)
{
    std::lock_guard<std::mutex> lck(mtx);
    q.push_back(msg);
    cond.notify_one();
}


void Synch_queue::put_first(const Message& msg)
{
    std::lock_guard<std::mutex> lck(mtx);
    q.push_front(msg);
    cond.notify_one();
}


void Synch_queue::get (Message& msg)
{
    std::unique_lock<std::mutex> lck(mtx);
    cond.wait(lck, [this]{return !q.empty();});
    msg = q.front();        //returns a reference to the first element in the list container.
    q.pop_front();          //removes the first element in the list container.
    
}
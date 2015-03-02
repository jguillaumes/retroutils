/* 
 * File:   BasicHandler.cpp
 * Author: jguillaumes
 * 
 * This handler keeps a tally of the seen nodes according to their type,
 * and lists what it has found every 10 seconds 
 * 
 * Created on 31 / gener / 2015, 22:43
 */

#include <cstdlib>
#include <map>
#include <iostream>
#include <sys/time.h>
#include "CounterHandler.h"

using namespace std;

/**
 * Constructor: zero the counters and initialize the timer
 */
CounterHandler::CounterHandler() {
    struct timeval tv;
    routers_l1 = 0;
    routers_l2 = 0;
    endnodes = 0;
    gettimeofday(&tv,NULL);
    timer = tv.tv_sec;
}

/*
 * Destructor: dereference the node information
 */
CounterHandler::~CounterHandler() {
    for (map<int,NODEINFO>::iterator i = nodes.begin(); i != nodes.end(); ++i) {
        nodes.erase(i);
    }
}

/*
 * Packet handler
 */
bool CounterHandler::handleHello(const BYTE *packet) {
    struct hello_t *hello = (struct hello_t *) packet;
    time_t thistime;
    struct timeval tv;
    
    int cursize = nodes.size();             // Current DB size
    NODEINFO &ni = nodes[hello->dnAddr];    // If dnaddr does not exist...
    if (nodes.size() > cursize) {           // a new entry will be created automagically
        ni.address = hello->dnAddr;
        ni.nodetype = hello->nodeInfo.nodeType;
        switch(hello->nodeInfo.nodeType) {  // Accumulate depending on type
            case 1:
                ni.hellotimer = hello->u.router.helloTimer;       
                routers_l2++;
                break;
            case 2:
                ni.hellotimer = hello->u.router.helloTimer;       
                routers_l1++;
                break;
            default:
                ni.hellotimer = hello->u.endNode.helloTimer;       
                endnodes++;
        }
        std::cout << "Endnodes: " << endnodes << ", L1 routers: " << routers_l1 
                  << ", L2 routers: " << routers_l2 << endl;
    }
    /*
     * Check if it is time to dump the DB and do it if that is the case
     */
    gettimeofday(&tv, NULL);
    thistime = tv.tv_sec;
    if (thistime - timer >= 10) {
        timer = thistime;
        cout << "LEVEL 1 Routers" << endl;
        for (map<int, NODEINFO>::iterator i = nodes.begin(); i != nodes.end(); ++i) {
            NODEINFO &ni = i->second;
            if (ni.nodetype == 2) {
                cout << "Address: ";
                cout.width(2);
                cout.setf(std::ios::right, std::ios::adjustfield);
                cout << AREA(ni.address) << "." ;
                cout.width(4);
                cout.setf(std::ios::left, std::ios::adjustfield); 
                cout << NODE(ni.address) << ", timer: " << ni.hellotimer << endl;
            }
        }
        cout << "LEVEL 2 Routers" << endl;
        for (map<int, NODEINFO>::iterator i = nodes.begin(); i != nodes.end(); ++i) {
            NODEINFO &ni = i->second;
            if (ni.nodetype == 1) {
                cout << "Address: ";
                cout.width(2);
                cout.setf(std::ios::right, std::ios::adjustfield);
                cout << AREA(ni.address) << "." ;
                cout.width(4);
                cout.setf(std::ios::left, std::ios::adjustfield); 
                cout << NODE(ni.address) << ", timer: " << ni.hellotimer << endl;
            }
        }
        cout << "Endnodes" << endl;
        for (map<int, NODEINFO>::iterator i = nodes.begin(); i != nodes.end(); ++i) {
            NODEINFO &ni = i->second;
            if (ni.nodetype == 3) {
                cout << "Address: ";
                cout.width(2);
                cout.setf(std::ios::right, std::ios::adjustfield);
                cout << AREA(ni.address) << "." ;
                cout.width(4);
                cout.setf(std::ios::left, std::ios::adjustfield); 
                cout << NODE(ni.address) << ", timer: " << ni.hellotimer << endl;
            }
        }
    }
    return true;
}



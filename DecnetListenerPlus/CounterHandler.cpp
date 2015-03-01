/* 
 * File:   BasicHandler.cpp
 * Author: jguillaumes
 * 
 * Created on 31 / gener / 2015, 22:43
 */

#include <cstdlib>
#include <map>
#include <iostream>

#include "CounterHandler.h"

using namespace std;

CounterHandler::CounterHandler() {}

CounterHandler::~CounterHandler() {
    for (map<int,int>::iterator i = nodes.begin(); i != nodes.end(); ++i) {
        nodes.erase(i);
    }
}

bool CounterHandler::handleHello(const BYTE *packet) {
    struct hello_t *hello = (struct hello_t *) packet;
    std::string type;

    type = typeName(hello -> nodeInfo.nodeType);
    std::cout << "HELLO packet from ";
    std::cout.width(2);
    std::cout.setf(std::ios::right, std::ios::adjustfield);
    std::cout << AREA(hello -> dnAddr) << ".";
    std::cout.width(4);
    std::cout.setf(std::ios::left, std::ios::adjustfield); 
    std::cout << NODE(hello -> dnAddr) << ", type=" ;
    std::cout.width(14);
    std::cout << type 
              << "(" << hello->nodeInfo.nodeType << ")" 
              << ", rtype=(" << hello->routingFlags.type << ")";
    if (hello -> nodeInfo.nodeType == 3) {
        std::cout << " Timer: " << hello->u.endNode.helloTimer;
        // printf(" Timer: %d DR: %d.%d\n", hello->u.endNode.helloTimer,AREA(dnFromMac(hello -> u.endNode.designatedRouter)), NODE(dnFromMac(hello -> u.endNode.designatedRouter)));
    } else {
        std::cout << " Timer: " << hello->u.router.helloTimer; 
        // printf(" Timer: %d Prio: %d\n", hello->u.router.helloTimer, hello->u.router.priority);
    }
    std::cout << std::endl;
    return true;
}

/*
 *  Translate the routing message type code to its name.
 */
const std::string CounterHandler::typeName(unsigned int nodeType) {
   std::string type;
    
    switch (nodeType) {
        case 1:
            type = "LEVEL 2 ROUTER";
            break;

        case 2:
            type = "LEVEL 1 ROUTER";
            break;

        case 3:
            type = "END NODE";
            break;

        default:
            type = "*UNKNOWN*";
    }

    return type;
}


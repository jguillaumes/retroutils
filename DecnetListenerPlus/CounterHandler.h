/* 
 * File:   CounterHandler.h
 * Author: jguillaumes
 *
 * Node counter handler
 * 
 * This implementation just handles HELLO packets, and counts the number of
 * nodes of each type (endnodes, l1 and l2 routers).
 * 
 * Created on 31 / gener / 2015, 22:43
 */

#ifndef COUNTERHANDLER_H
#define	COUNTERHANDLER_H

#include <string>
#include <map>
#include "DecnetDefs.h"
#include "PacketHandler.h"

class CounterHandler : public PacketHandler {
public:
    CounterHandler();
    virtual ~CounterHandler();
    /*
     * Accumulate depending on hello type, return true.
     */
    virtual bool handleHello(const BYTE* packet);

private:
    std::map<int, NODEINFO> nodes;  // Node database
    int endnodes;                   // Number of endnodes
    int routers_l1;                 // Number of L1 routers
    int routers_l2;                 // Number of L2 routers
    time_t timer;                   // Last time we dumped the list of nodes
};

#endif	/* COUNTERHANDLER_H */


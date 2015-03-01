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

#include "DecnetDefs.h"
#include "PacketHandler.h"

class CounterHandler : public PacketHandler {
public:
    CounterHandler();
    virtual ~CounterHandler();
    /*
     * Display the read packet, return true.
     */
    virtual bool handleHello(const BYTE* packet);

private:
    /*
     * Translate type of packet to a human readable string
     */
    std::map<int, int> nodes;
    int endnodes;
    int routers_l1;
    int routers_l2;
    const std::string typeName(unsigned int nodetype);
};

#endif	/* COUNTERHANDLER_H */


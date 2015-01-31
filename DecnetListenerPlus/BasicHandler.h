/* 
 * File:   BasicHandler.h
 * Author: jguillaumes
 *
 * Basic implementation of a Packet Handler
 * 
 * This implementation just handles HELLO packets, and prints to stdout
 * a line describing the read packet.
 * 
 * Created on 31 / gener / 2015, 22:43
 */

#ifndef BASICHANDLER_H
#define	BASICHANDLER_H

#include "DecnetDefs.h"
#include "PacketHandler.h"

class BasicHandler : public PacketHandler {
public:
    BasicHandler() {};
    virtual ~BasicHandler() {}
    /*
     * Display the read packet, return true.
     */
    virtual bool handleHello(const BYTE* packet);

private:
    /*
     * Translate type of packet to a human readable string
     */
    const std::string typeName(unsigned int nodetype);

};

#endif	/* BASICHANDLER_H */


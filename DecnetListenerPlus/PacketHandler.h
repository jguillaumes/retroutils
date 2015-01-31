/* 
 * File:   PacketHandler.h
 * Author: jguillaumes
 *
 * Created on 31 / gener / 2015, 22:25
 */

#ifndef PACKETHANDLER_H
#define	PACKETHANDLER_H

#include "DecnetDefs.h"

class PacketHandler {
public:
    PacketHandler() {};
    virtual ~PacketHandler() {};
    
    virtual void handleIdle() {};
    virtual bool handleHello(const BYTE *packet) { return false; }
    virtual bool handleInit(const BYTE *packet) { return false; }
    virtual bool handleRouting(const BYTE *packet) { return false; }
    virtual bool handleUnknown(const BYTE *packet) { return false; }
    
private:

};

#endif	/* PACKETHANDLER_H */


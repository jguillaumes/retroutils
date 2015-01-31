/* 
 * File:   PacketHandler.h
 * Author: jguillaumes
 *
 * Handler for captured DECNET routing (ethertype 0x6003) packets.
 * 
 * This class provides methods to be called by DecnetListener when it captures
 * a DECNET IV routing protocol packet. The default implementation does nothing.
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
    
    /*
     * handleIdle is called at every turn of the handling loop, so the application
     * can handle adjacency changes and process routing table messages.
     */
    virtual void handleIdle() {};
    /*
     * handleHello is called for each HELLO packet. Returns true if the
     * packet has been handled. The default implementation does nothing, so it
     * returns always false.
     */
    virtual bool handleHello(const BYTE *packet) { return false; }
    /*
     * handleHello is called for each INIT packet. Returns true if the
     * packet has been handled. The default implementation does nothing, so it
     * returns always false.
     */
    virtual bool handleInit(const BYTE *packet) { return false; }
    /*
     * handleHello is called for each ROUTING packet. Returns true if the
     * packet has been handled. The default implementation does nothing, so it
     * returns always false.
     */
    virtual bool handleRouting(const BYTE *packet) { return false; }
    /*
     * handleHello is called for each unknown packet. Returns true if the
     * packet has been handled. The default implementation does nothing, so it
     * returns always false.
     */
    virtual bool handleUnknown(const BYTE *packet) { return false; }
    
private:

};

#endif	/* PACKETHANDLER_H */


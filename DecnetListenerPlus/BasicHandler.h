/* 
 * File:   BasicHandler.h
 * Author: jguillaumes
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
    virtual bool handleHello(const BYTE* packet);

private:
    const std::string typeName(unsigned int nodetype);

};

#endif	/* BASICHANDLER_H */


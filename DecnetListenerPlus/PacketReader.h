/* 
 * File:   PacketReader.h
 * Author: jguillaumes
 *
 * Created on 29 / gener / 2015, 10:59
 */

#ifndef PACKETREADER_H
#define	PACKETREADER_H

#include <cstdlib>

#include "DecnetDefs.h"

class PacketReader {
public:
    PacketReader() {};
    virtual ~PacketReader() {};
    virtual const BYTE *capturePacket(int &size) = 0;
    const std::string getMsgError() { return msgError; };
    bool isCapturing() {
        return capturing;
    };

protected:
    bool capturing;
    std::string msgError;
};

#endif	/* PACKETREADER_H */


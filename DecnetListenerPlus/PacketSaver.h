/* 
 * File:   PacketSaver.h
 * Author: jguillaumes
 *
 * Created on 29 / gener / 2015, 11:02
 */

#ifndef PACKETSAVER_H
#define	PACKETSAVER_H

class PacketSaver {
public:
    PacketSaver() {};
    virtual ~PacketSaver() {};
    virtual bool isSaving() { return false; };
    virtual const std::string getMsgError() { return ""; };
    virtual void savePacket(const BYTE *packet, int size) {};

};

#endif	/* PACKETSAVER_H */


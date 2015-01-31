/* 
 * File:   PcapReader.h
 * Author: jguillaumes
 *
 * Created on 29 / gener / 2015, 11:11
 */

#ifndef PCAPREADER_H
#define	PCAPREADER_H

#include <string>
#include "PacketReader.h"

class PcapReader : public PacketReader {
public:
    PcapReader(const std::string& iface);
    virtual ~PcapReader();
    virtual const BYTE *capturePacket(int &size);
    
private:
    pcap_t *handle;
    const static std::string capFilter;
};

#endif	/* PCAPREADER_H */


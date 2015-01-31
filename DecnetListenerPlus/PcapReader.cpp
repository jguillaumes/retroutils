/* 
 * File:   PcapReader.cpp
 * Author: jguillaumes
 * 
 * Created on 29 / gener / 2015, 11:11
 */

#include <cstdlib>
#include <sstream>

#include <pcap.h>

#include "PcapReader.h"

const std::string PcapReader::capFilter = "ether host AB-00-00-03-00-00";

PcapReader::PcapReader(const std::string& iface) {
    struct bpf_program pf;

    char error[64];
    char buff[128];
    std::ostringstream ostr;
    ostr.str(buff);

    /*
     * Open the packet capture (libpcap) and prepare the capture
     * filter. The capture filter specifies to get all the traffic
     * sent to the DECNET routing multicast address.
     */
    handle = pcap_open_live(iface.c_str(), 512, 1, 0, error);

    if (handle == NULL) {
        ostr << "Error opening device " << iface << ": (" << error << ")"; 
        msgError = ostr.str();
        return;
    }

    if (-1 == pcap_compile(handle, &pf, capFilter.c_str(), 1, 0)) {
        ostr << "Error compiling the filter expression (" <<  pcap_geterr(handle) << ")";
        msgError = ostr.str();
        return;
    }

    if (-1 == pcap_setfilter(handle, &pf)) {
        ostr << "Error setting up filter (" << pcap_geterr(handle) << ")";
        msgError = ostr.str();
        return;
    }
    capturing = true;
}

PcapReader::~PcapReader() {
    if (capturing) {
        pcap_close(handle);
        capturing = false;
    }
}

const BYTE *PcapReader::capturePacket(int& size) {
    struct pcap_pkthdr header;
    const BYTE *packet;
    packet = pcap_next(handle,&header);
    size = header.len;
    return packet;
}
 

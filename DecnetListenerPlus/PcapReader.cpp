/* 
 * File:   PcapReader.cpp
 * Author: jguillaumes
 * 
 * PCAP based implementation of a packet reader
 * 
 * Created on 29 / gener / 2015, 11:11
 */

#include <cstdlib>
#include <sstream>

#include <pcap.h>

#include "PcapReader.h"

const std::string PcapReader::capFilter = "ether host AB-00-00-03-00-00";

/*
 * Constructor: prepare the pcap capture. The capture interface has 
 * to be passed as parameter.
 */
PcapReader::PcapReader(const std::string& iface) {
    struct bpf_program pf;

    char error[64];             // Buffer to build error messages
    char buff[128];             // I/O buffer
    std::ostringstream ostr;    // Stream to build error messages
    ostr.str(buff);             // Set it up to char buffer

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

    /*
     * Compile the filter expression. It should capture the packets
     * addressed to the decnet routing multicast address (AB:00:00:03:00:00)
     */
    if (-1 == pcap_compile(handle, &pf, capFilter.c_str(), 1, 0)) {
        ostr << "Error compiling the filter expression (" <<  pcap_geterr(handle) << ")";
        msgError = ostr.str();
        return;
    }

    /*
     * Apply the compiled filter
     */
    if (-1 == pcap_setfilter(handle, &pf)) {
        ostr << "Error setting up filter (" << pcap_geterr(handle) << ")";
        msgError = ostr.str();
        return;
    }
    capturing = true;
}

/*
 * Destructor: close the pcap capture
 */
PcapReader::~PcapReader() {
    if (capturing) {
        pcap_close(handle);
        capturing = false;
    }
}

/*
 * Get next ethernet packet
 * This implementation waits for a packet to be read
 * (The timeout is set up at open() time, in the constructor)
 * 
 */
const BYTE *PcapReader::capturePacket(int& size) {
    struct pcap_pkthdr header;
    const BYTE *packet;
    packet = pcap_next(handle,&header);
    size = header.len;
    return packet;
}
 

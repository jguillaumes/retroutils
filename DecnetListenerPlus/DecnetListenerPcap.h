/* 
 * File:   DecnetListener.h
 * Author: jguillaumes
 *
 * Created on 28 / gener / 2015, 13:40
 */

#ifndef DECNETLISTENERPCAP_H
#define	DECNETLISTENERPCAP_H

#include "DecnetFileSaver.h"


class DecnetListenerPcap :  public DecnetFileSaver {
public:
    DecnetListenerPcap(const std::string& iface, const std::string &fileName);
    ~DecnetListenerPcap();

private:
    pcap_t *handle;

protected:
    const static std::string capFilter;
    const virtual BYTE *capturePacket(int &size);
};


#endif	/* DECNETLISTENERPCAP_H */


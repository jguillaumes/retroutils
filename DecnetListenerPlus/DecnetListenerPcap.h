/* 
 * File:   DecnetListener.h
 * Author: jguillaumes
 *
 * Created on 28 / gener / 2015, 13:40
 */

#ifndef DECNETLISTENERPCAP_H
#define	DECNETLISTENERPCAP_H

#include "FileSaver.h"


class DecnetListenerPcap :  public DecnetListener {
public:
    DecnetListenerPcap(const std::string& iface, const std::string &fileName);
    ~DecnetListenerPcap();

private:
    
protected:
};


#endif	/* DECNETLISTENERPCAP_H */


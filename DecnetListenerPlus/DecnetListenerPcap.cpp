/* 
 * File:   DecnetListener.cpp
 * Author: jguillaumes
 * 
 * Created on 28 / gener / 2015, 13:40
 */

#include <sstream>
#include <fstream>

#include <pcap.h>
#include <errno.h>

#include "DecnetDefs.h"
#include "DecnetListener.h"
#include "DecnetListenerPcap.h"
#include "BasicHandler.h"
#include "FileSaver.h"
#include "PcapReader.h"
#include "BasicHandler.h"

using namespace std;

DecnetListenerPcap::DecnetListenerPcap(const std::string& iface, 
                                       const std::string& fileName) {
    packetReader = new PcapReader(iface);
    packetHandler = new BasicHandler();
    if (packetReader->isCapturing()) {
        packetSaver = new FileSaver(fileName);
        if (!packetSaver->isSaving()) {
            msgError = errno;
        }
    } else {
        msgError = packetReader->getMsgError();
    }
}

DecnetListenerPcap::~DecnetListenerPcap() {
    delete packetReader;
}

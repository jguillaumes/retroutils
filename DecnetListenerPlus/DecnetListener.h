/* 
 * File:   DecnetListener.h
 * Author: jguillaumes
 *
 * Created on 28 / gener / 2015, 21:21
 */
#ifndef DECNETLISTENER_H
#define	DECNETLISTENER_H

#include <string>

#include "PacketReader.h"
#include "PacketSaver.h"
#include "PacketHandler.h"

class DecnetListener {
private:
    bool saveAll;
    bool cont;
    bool doCmds;
    
protected:
    PacketReader  *packetReader;
    PacketSaver   *packetSaver;
    PacketHandler *packetHandler;
    std::string msgError;
    
public:
    DecnetListener() { 
        packetSaver = new PacketSaver(); 
        packetHandler = new PacketHandler();
    };

    DecnetListener(PacketReader *reader, 
                   PacketSaver *saver,
                   PacketHandler *handler) {
        packetSaver = saver;
        packetReader = reader;
        packetHandler = handler;
    }
    virtual ~DecnetListener() { 
        delete packetSaver;
        delete packetHandler;
    }

    virtual bool isSaving() { return false; }
    
    std::string getMsgError() {
        return msgError;
    };
    void setPacketReader(PacketReader *reader) { packetReader = reader; };
    void setPacketHandler(PacketHandler *handler) { packetHandler = handler; }
    void setSaveAll(bool saveAll) { this->saveAll = saveAll; };
    bool captureLoop();
    bool isCapturing() {
        return packetReader->isCapturing();
    }
};

#endif	/* DECNETLISTENER_H */


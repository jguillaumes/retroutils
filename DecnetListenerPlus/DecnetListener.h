/* 
 * File:   DecnetListener.h
 * Author: jguillaumes
 *
 * Created on 28 / gener / 2015, 21:21
 */
#ifndef DECNETLISTENER_H
#define	DECNETLISTENER_H

#include <string>

class DecnetListener {
protected:
    bool capturing;
    bool saveAll;
    bool cont;
    bool doCmds;
    std::string msgError;
    virtual const BYTE *capturePacket(int &size) {};
    virtual void savePacket(const BYTE *packet, int size) = 0;
    
public:
    DecnetListener();
    virtual ~DecnetListener();

    virtual bool isSaving() { return false; }
    
    bool isCapturing() {
        return capturing;
    };
    std::string getMsgError() {
        return msgError;
    };
    bool setCapFile(const std::string& filename);
    void setSaveAll(bool saveAll);
    bool captureLoop(void(*callback)(hello_t*));
};

#endif	/* DECNETLISTENER_H */


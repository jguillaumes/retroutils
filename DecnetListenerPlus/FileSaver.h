/* 
 * File:   DecnetFileSaver.h
 * Author: jguillaumes
 *
 * Created on 28 / gener / 2015, 23:17
 */

#ifndef FILESAVER_H
#define	FILESAVER_H

#include <string>

#include "DecnetDefs.h"
#include "DecnetListener.h"

using namespace std;

class FileSaver : public PacketSaver {
public:
    FileSaver(const std::string &filename);
    virtual ~FileSaver();
    bool isFileOpened() { return fileOpened; }
    bool virtual isSaving() { return isFileOpened(); }
    virtual const std::string getMsgError() { return msgError; };
    virtual void savePacket(const BYTE *packet, int size);
    
private:
    std::string msgError;
    std::ofstream capfile;
    bool fileOpened;
};

#endif	/* FILESAVER_H */

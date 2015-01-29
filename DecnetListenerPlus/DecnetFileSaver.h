/* 
 * File:   DecnetFileSaver.h
 * Author: jguillaumes
 *
 * Created on 28 / gener / 2015, 23:17
 */

#ifndef DECNETFILESAVER_H
#define	DECNETFILESAVER_H

#include <string>

#include "DecnetDefs.h"
#include "DecnetListener.h"

using namespace std;

class DecnetFileSaver : public DecnetListener {
public:
    DecnetFileSaver(const std::string &filename);
    virtual ~DecnetFileSaver();
    bool isFileOpened() { return fileOpened; }
    bool virtual isSaving() { return isFileOpened(); }

protected:
    virtual void savePacket(const BYTE *packet, int size);
    
private:
    std::ofstream capfile;
    bool fileOpened;
};

#endif	/* DECNETFILESAVER_H */


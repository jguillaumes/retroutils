/* 
 * File:   DecnetListener.cpp
 * Author: jguillaumes
 * 
 * Created on 28 / gener / 2015, 21:21
 */

#include <cstddef>

#include "DecnetDefs.h"
#include "DecnetListener.h"

using namespace std;

DecnetListener::DecnetListener() {
    capturing = false;
}

DecnetListener::~DecnetListener() {
}


void DecnetListener::setSaveAll(bool saveAll) {
    this->saveAll = saveAll;
}

bool DecnetListener::captureLoop(void(*callback)(hello_t*)) {

    bool result = true;
    bool save = false;
    int size;
    struct frame_s *frame;
    struct hello_t *hello;
    WORD etherType;
    const BYTE *packet;
    
    cont = true;
    while(cont) {
        save = saveAll;
        packet = capturePacket(size);
        frame = (struct frame_s *) packet;
        etherType = frame->u.nonTagged.etherType;
        hello = (struct hello_t *) (packet + OFS_FRAME);
        if (etherType == ET_VLANTAG) {
            etherType = frame->u.tagged.etherType;
            hello = (struct hello_t *) ((BYTE *)hello + 2);
        }
        if (etherType == ET_DNETROUTING) {
            
            if ((*((BYTE *)hello) & 0x80) == 0x80) {
                hello = (struct hello_t *) (((BYTE *)hello) + 1);
            }
         /*
         * Now, if hello is not null then it points to the hello
         * message and we can handle it.
         */
            if (hello != NULL) {
                switch (hello -> routingFlags.type) {
                    case 6:         /* Endnode Hello */
                    case 5:         /* Router Hello  */
                        callback(hello);
                        break;
                    case 0:         /* Route init message */
                        break;
                    case 4:         /* Level 2 routing message */
                    case 3:         /* Level 1 routing message */
                        break;
                    default:        /* Unknown message: log it if requested */                        
                        save = isSaving();
                        break;
                }
                if (save) savePacket(packet, size);
            }
        }
    }
    return result;
}


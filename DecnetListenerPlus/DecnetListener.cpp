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

bool DecnetListener::captureLoop() {

    bool result = true;
    bool save = false;
    bool handled = false;
    int size;
    struct frame_s *frame;
    struct hello_t *hello;
    WORD etherType;
    const BYTE *packet;
    
    cont = true;
    while(cont) {
        save = saveAll;
        handled = false;
        packetHandler->handleIdle();
        packet = packetReader->capturePacket(size);
        frame = (struct frame_s *) packet;
        etherType = frame->u.nonTagged.etherType;
        packet += OFS_FRAME;
        hello = (struct hello_t *) packet;
        if (etherType == ET_VLANTAG) {
            etherType = frame->u.tagged.etherType;
            packet += 2;
            hello = (struct hello_t *) packet;
        }
        if (etherType == ET_DNETROUTING) {
            
            if ((*((BYTE *)hello) & 0x80) == 0x80) {
                packet += 1;
                hello = (struct hello_t *) packet;
            }
         /*
         * Now, if hello is not null then it points to the hello
         * message and we can handle it.
         */
            if (hello != NULL) {
                switch (hello -> routingFlags.type) {
                    case 6:         /* Endnode Hello */
                    case 5:         /* Router Hello  */
                        handled = packetHandler->handleHello(packet);
                        break;
                    case 0:         /* Route init message */
                        handled = packetHandler->handleInit(packet);
                        break;
                    case 4:         /* Level 2 routing message */
                    case 3:         /* Level 1 routing message */
                        handled = packetHandler->handleRouting(packet);
                        break;
                    default:        /* Unknown message: log it if requested */                        
                        handled = packetHandler->handleUnknown(packet);
                        break;
                }             
                if (save) {
                    if (saveAll || !handled) {
                        packetSaver->savePacket((BYTE *)frame, size);                        
                    }
                }
            }
        }
    }
    return result;
}


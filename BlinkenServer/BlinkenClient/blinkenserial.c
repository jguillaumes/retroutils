/* blinkenserial.c: BlinkenLights for the PDP11
 ------------------------------------------------------------------------------
 Copyright (c) 2013, Jordi Guillaumes Pons
 
 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 Except as contained in this notice, the name of the author shall not be
 used in advertising or otherwise to promote the sale, use or other dealings
 in this Software without prior written authorization from the author.
 
 -----------------------------------------------------------------------------
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "blinkenclient.h"
#include "blinkenserial.h"

#define XON  0x11
#define XOFF 0x13

static int doSerialWrite(PBLINKENSTATUS pb, PPAYLOAD buffer, int size) {
    int rc=0;
    /*
    char charin;
    
    rc = (int) read(pb->conn.tty_status.fileDesc,&charin,1);
    if (rc == 1) {
        switch(charin) {
            case XON:
                if (pb->conn.tty_status.commStatus != BLKC_OK) {
                    fprintf(stderr, "Got XON from device, traffic resuming\n");
                }
                pb->conn.tty_status.commStatus = BLKC_OK;
                break;
            case XOFF:
                if (pb->conn.tty_status.commStatus == BLKC_OK) {
                    fprintf(stderr, "Got XOFF from device, please easy down the rate.\n");
                }
                pb->conn.tty_status.commStatus = BLKC_HELD;
                break;
            default:
                fprintf(stderr, "Unexpected byte got from device (%d)\n", charin);
        }
    }
    */
    if (pb->conn.tty_status.commStatus == BLKC_OK) {
        return (int) write(pb->conn.tty_status.fileDesc, buffer, size);
    } else {
        fprintf(stderr, "Skipped packet due to device XOFF\n");
        return size;        // Simulate a correct write
    }
    return rc;
}


PBLINKENSTATUS blk_serialOpen(char *dev) {
    PBLINKENSTATUS blkstatus = NULL;
    struct termios tio;
    int fd = 0;
    
    memset(&tio, 0, sizeof(struct termios));
    cfmakeraw(&tio);
    cfsetspeed(&tio, B230400);
    
    fd = open(dev, O_RDWR | O_NOCTTY);
    if (fd > 0) {
        /*
         * Set to RAW mode
         */
        if (tcsetattr(fd, TCSANOW, &tio) < 0) {
            perror("blk_open");
            close(fd);
            fd = -1;
        } else {
            /*
             * Set to non-blocking I/O
             */
            /*
            if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
                perror("blk_open");
                close(fd);
                fd = -1;
            }
            */
        }
    }
    if (fd < 0) return NULL;
    blkstatus = malloc(sizeof(BLINKENSTATUS));
    if (blkstatus == NULL) {
        close(fd);
    } else {
        memset(blkstatus, 0, sizeof(BLINKENSTATUS));
        blkstatus->conntype = BLKT_SERIAL;
        blkstatus->conn.tty_status.fileDesc = fd;
        strncpy(blkstatus->conn.tty_status.ttyName, dev, _POSIX_PATH_MAX);
    }
    return blkstatus;
}

int blk_serialSendByte(PBLINKENSTATUS pbstat, BYTE b, int flags) {
    PAYLOAD payload;
    int rc = 0;
    
    memset(&payload, 0, sizeof(PAYLOAD));
    payload.bflags = htons(flags);
    payload.numDataBytes = htons(2);
    payload.data[0] = b;
    
    /*
     * Non-blocking write
     * If the device is not ready to get the block errno will be set
     * to EAGAIN. We will ignore this error (we don't mind if we lose
     * some blinkenlights change...)
     */
    if (sizeof(PAYLOAD) != doSerialWrite(pbstat, &payload, sizeof(PAYLOAD))) {
        if (errno != EAGAIN) rc = -1;
    }
    return rc;
}

int blk_serialSendWord(PBLINKENSTATUS pbstat, WORD w, int flags) {
    unsigned short theword = 0;
    unsigned char *thebytes;
    PAYLOAD payload;
    int rc = 0;
    
    theword = htons(w);
    thebytes = (unsigned char *)&theword;
    
    memset(&payload, 0, sizeof(PAYLOAD));
    payload.bflags = htons(flags);
    payload.numDataBytes = htons(2);
    payload.data[0] = thebytes[0];
    payload.data[1] = thebytes[1];
    
    if (sizeof(PAYLOAD) == doSerialWrite(pbstat, &payload, sizeof(PAYLOAD))) {
        rc = 0;
    } else {
        rc = -1;
    }
    return rc;
}

int blk_serialSendError(PBLINKENSTATUS pbstat) {
    PAYLOAD payload;
    int rc = 0;
    
    memset(&payload, 0, sizeof(PAYLOAD));
    payload.bflags = htons(BLF_ERROR);
    payload.numDataBytes = htons(2);
    
    if (sizeof(PAYLOAD) == doSerialWrite(pbstat, &payload, sizeof(PAYLOAD))) {
        rc = 0;
    } else {
        rc = -1;
    }
    return rc;
}

void blk_serialClose(PBLINKENSTATUS pblk) {
    close(pblk->conn.tty_status.fileDesc);
    free(pblk);
}


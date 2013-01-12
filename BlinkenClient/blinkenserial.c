#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "blinkenclient.h"
#include "blinkenserial.h"

PBLINKENSTATUS blk_serialOpen(char *dev) {
    PBLINKENSTATUS blkstatus = NULL;
    struct termios tio;
    int fd = 0;
    
    memset(&tio, 0, sizeof(struct termios));
    cfmakeraw(&tio);
    cfsetspeed(&tio, B57600);
    
    fd = open(dev, O_RDWR | O_NOCTTY);
    if (fd > 0) {
        if (tcsetattr(fd, TCSANOW, &tio) < 0) {
            perror("blk_open");
            close(fd);
            fd = -1;
        }
    }
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
    
    if (sizeof(PAYLOAD) == write(pbstat->conn.tty_status.fileDesc, &payload, sizeof(PAYLOAD))) {
        rc = 0;
    } else {
        rc = -1;
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
    
    if (sizeof(PAYLOAD) == write(pbstat->conn.tty_status.fileDesc, &payload, sizeof(PAYLOAD))) {
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
    
    if (sizeof(PAYLOAD) == write(pbstat->conn.tty_status.fileDesc, &payload, sizeof(PAYLOAD))) {
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


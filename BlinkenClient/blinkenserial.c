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

int blk_serialOpen(char *dev) {
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
  return fd;
}

int blk_serialClose(int fd) {
  return close(fd);
}

int blk_serialSendWord(int fd, int value, int flags) {
    unsigned short theword = 0;
    unsigned char *thebytes;
    PAYLOAD payload;
    int rc = 0;

    theword = htons(value);
    thebytes = (unsigned char *)&theword;
    
    memset(&payload, 0, sizeof(PAYLOAD));
    payload.bflags = htons(flags);
    payload.numDataBytes = htons(2);
    payload.data[0] = thebytes[0];
    payload.data[1] = thebytes[1];
    
    if (sizeof(PAYLOAD) == write(fd, &payload, sizeof(PAYLOAD))) {
        rc = 0;
    } else {
        rc = -1;
    }
    return rc;
}

int blk_serialSendError(int fd) {
    PAYLOAD payload;
    int rc = 0;
    
    memset(&payload, 0, sizeof(PAYLOAD));
    payload.bflags = htons(BLF_ERROR);
    payload.numDataBytes = htons(2);
    
    if (sizeof(PAYLOAD) == write(fd, &payload, sizeof(PAYLOAD))) {
        rc = 0;
    } else {
        rc = -1;
    }
    return rc;    
}


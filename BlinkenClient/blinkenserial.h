#ifndef _BLINKSERIAL_H

int blk_serialOpen(char *dev);
int blk_serialClose(int fd);
int blk_serialSendWord(int fd, int value, int flags);
int blk_serialSendError(int fd);

#define _BLINKSERIAL_H
#endif

#ifndef _BLINKSERIAL_H

#define BLKC_OK      0x0000
#define BLKC_HELD    0x0001
#define BLKC_STALLED 0x0002

struct tty_status_s {
    char    ttyName[_POSIX_PATH_MAX];
    int     fileDesc;
    int     commStatus;
};

typedef struct tty_status_s TTYSTATUS;
typedef struct tty_status_s *PTTYSTATUS;

#define _BLINKSERIAL_H
#endif

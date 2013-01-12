#ifndef _BLINKSERIAL_H

struct tty_status_s {
    char    ttyName[_POSIX_PATH_MAX];
    int     fileDesc;
};

typedef struct tty_status_s TTYSTATUS;
typedef struct tty_status_s *PTTYSTATUS;

#define _BLINKSERIAL_H
#endif

//
//  BlinkenTestClient.c
//  BlinkenServer
//
//  Created by Jordi Guillaumes Pons on 01/01/13.
//  Copyright (c) 2013 Jordi Guillaumes Pons. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "blinkenclient.h"
#include "blinkenserial.h"

int main(int argc, char **argv) {
    char *ttyname= argv[1];
    int delay = atoi(argv[2]);
    int i=0;
    int fd=0;
    
    printf("Starting\n");
    fd = blk_serialOpen(ttyname);
    if (fd > 0) {
        for(i=0;i<65536;i++) {
            blk_serialSendWord(fd, i, 0);
            usleep(delay);
            blk_serialSendError(fd);
            usleep(delay);
        }
        blk_serialClose(fd);
    } else {
        perror("BlinkenTestSerial");
    }
}
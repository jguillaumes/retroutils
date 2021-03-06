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


int main(int argc, char **argv) {
    PBLINKENSTATUS pb=NULL;
    char *conname = argv[1];
    int delay = atoi(argv[2]);
    
    int i=0;
    int resync=1;
    
    pb=blk_open(conname);
    if (pb!=NULL) {
        for(i=0;i<65536;i++) {
            blk_sendWord(pb, (WORD) i, resync);
            resync=0;
            usleep(delay);
//            blk_sendError(pb, resync);
//            usleep(delay);
        }
        blk_close(pb);
    } else {
        perror("BlinkenTestClient");
    }
}

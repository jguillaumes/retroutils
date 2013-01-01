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
    char *hostname = argv[1];
    WORD port = atoi(argv[2]);
    int delay = atoi(argv[3]);
    int i=0;
    int resync=1;
    
    pb=blk_setup(hostname, port);
    if (pb!=NULL) {
        for(i=0;i<65536;i++) {
            blk_sendword(pb, (WORD) i, resync);
            resync=0;
            usleep(delay);
        }
        blk_close(pb);
    }
}
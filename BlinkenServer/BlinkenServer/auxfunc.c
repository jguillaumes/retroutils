//
//  auxfunc.c
//  BlinkenServer
//
//  Created by Jordi Guillaumes Pons on 01/01/13.
//  Copyright (c) 2013 Jordi Guillaumes Pons. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "blinkenserver.h"

int dotip(int address, char *buffer, int bufsiz) {
    BYTE *bytes;
    
    if (bufsiz < 16) return -1;

    memset(buffer, 0, bufsiz);
    bytes = (BYTE *) &address;
    sprintf(buffer,"%u.%u.%u.%u", bytes[0], bytes[1], bytes[2], bytes[3] );
    return 0;
}

int dobinary(BYTE byte, char *buffer, int bufsiz) {
    int i=0;
    
    if (bufsiz != 9) return -1;
    
    memset(buffer, 0, bufsiz);
    for(i=0; i<8; i++) { 
        buffer[i] = byte & 0x80 ? '1' : '0';
        byte <<= 1;
    }
    return(0);
}
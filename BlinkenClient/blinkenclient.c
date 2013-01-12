//
//  blinkenclient.c
//  BlinkenServer
//
//  Created by Jordi Guillaumes Pons on 12/01/13.
//  Copyright (c) 2013 Jordi Guillaumes Pons. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "blinkenclient.h"

static void allUpper(char *string) {
    char *ptr = string;
    while(*ptr != '\0') {
        *ptr = toupper(*ptr);
        ptr++;
    }
}


PBLINKENSTATUS blk_open(char *connString) {
    char *pconntype = NULL;
    char *pname = NULL;
    char *pnum = NULL;
    char *buff = NULL;
    int lConnString = 0;

    lConnString = (int) strlen(connString);
    buff = alloca(lConnString+1);
    memset(buff,0,lConnString+1);
    strncpy(buff, connString, lConnString);
    
    pconntype = strtok(buff, ":");
    if (pconntype == NULL) return NULL;
    allUpper(pconntype);
    
    pname = strtok(NULL,":");
    if (pname == NULL) return NULL;
    
    pnum = strtok(NULL, ":");
    
    if (strcmp(pconntype, "UDP") == 0) {
        return blk_udpOpen(pname, pnum);
    } else if (strcmp(pconntype, "TTY") == 0) {
        return blk_serialOpen(pname);
    } else {
        fprintf(stderr, "Invalid connection type (%s)\n", pconntype);
        return NULL;
    }
}

int blk_sendByte(PBLINKENSTATUS pblk, BYTE b, int resync) {
    switch(pblk->conntype) {
        case BLKT_SERIAL:
            return blk_serialSendByte(pblk, b, resync);
            break;
        case BLKT_UDP:
            return blk_udpSendByte(pblk, b, resync);
            break;
        default:
            fprintf(stderr, "Unsupported connection type (%d)\n", pblk->conntype);
            return -1;
    }
}

int blk_sendWord(PBLINKENSTATUS pblk, WORD w, int resync) {
    switch(pblk->conntype) {
        case BLKT_SERIAL:
            return blk_serialSendWord(pblk, w, resync);
            break;
        case BLKT_UDP:
            return blk_udpSendWord(pblk, w, resync);
            break;
        default:
            fprintf(stderr, "Unsupported connection type (%d)\n", pblk->conntype);
            return -1;
    }    
}

int blk_sendError(PBLINKENSTATUS pblk, int resync) {
    switch(pblk->conntype) {
        case BLKT_SERIAL:
            return blk_serialSendError(pblk);
            break;
        case BLKT_UDP:
            return blk_udpSendError(pblk, resync);
            break;
        default:
            fprintf(stderr, "Unsupported connection type (%d)\n", pblk->conntype);
            return -1;
    }    
}

void blk_close(PBLINKENSTATUS pblk) {
    switch(pblk->conntype) {
        case BLKT_SERIAL:
            blk_serialClose(pblk);
            break;
        case BLKT_UDP:
            blk_udpClose(pblk);
            break;
        default:
            fprintf(stderr, "Unsupported connection type (%d)\n", pblk->conntype);
    }
}

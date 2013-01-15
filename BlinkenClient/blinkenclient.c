/* blinkenclient.c: BlinkenLights for the PDP11
 ------------------------------------------------------------------------------
 Copyright (c) 2013, Jordi Guillaumes Pons
 
 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 Except as contained in this notice, the name of the author shall not be
 used in advertising or otherwise to promote the sale, use or other dealings
 in this Software without prior written authorization from the author.
 
 -----------------------------------------------------------------------------
 */

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

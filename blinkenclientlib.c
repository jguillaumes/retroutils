//
//  blinkenclientlib.c
//  BlinkenServer
//
//  Created by Jordi Guillaumes Pons on 01/01/13.
//  Copyright (c) 2013 Jordi Guillaumes Pons. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "blinkenclient.h"


PBLINKENSTATUS blk_setup(char *hostname, WORD portNumber) {
    PBLINKENSTATUS bkstat = NULL;
    struct hostent *serverHost = NULL;
    int udpsock = 0;
    struct protoent *udpproto;

    serverHost = gethostbyname(hostname);
    if (serverHost == NULL) {
        fprintf(stderr, "Host %s not found\n", hostname);
        return NULL;
    }
    udpproto = getprotobyname("udp");
    if (udpproto == NULL) {
        fprintf(stderr, "Error getting information for protocol udp\n");
        fprintf(stderr, "This is probably a configuration problem!\n");
        return NULL;
    }
    udpsock = socket(PF_INET, SOCK_DGRAM, udpproto->p_proto);
    if (udpsock < 0) {
        perror("getting socket");
        return NULL;
    }
    
    bkstat = malloc(sizeof(BLINKENSTATUS));
    if (bkstat == NULL) {
        fprintf(stderr, "Can't get memory to allocate client status!\n");
        return NULL;
    }
    memset(bkstat, 0, sizeof(BLINKENSTATUS));
    bkstat->sequence = 0;
    bkstat->socket = udpsock;
    bkstat->serverAddress.sin_family = AF_INET;
    bkstat->serverAddress.sin_port = portNumber;
    bkstat->serverAddress.sin_addr.s_addr = *((in_addr_t *)serverHost->h_addr_list[0]);
#ifdef OSX
    bkstat->serverAddress.sin_len
#endif
    return bkstat;
}

int blk_sendbyte(PBLINKENSTATUS pblk, BYTE b, int resync) {
    PAYLOAD payload;
    
    payload.sequence = ++(pblk->sequence);
    payload.numbits = 8;
    payload.flags = resync ? FLG_RESYNC : 0;
    payload.data[0] = b;
    return (int) sendto(pblk->socket, &payload, sizeof(payload), 0, (struct sockaddr *) &(pblk->serverAddress), sizeof(struct sockaddr));
}

int blk_sendword(PBLINKENSTATUS pblk, WORD w, int resync) {
    PAYLOAD payload;
    
    payload.sequence = ++(pblk->sequence);
    payload.numbits = 16;
    payload.flags = resync ? FLG_RESYNC : 0;
    payload.data[0] = w & 0xFF;
    payload.data[1] = (w >> 8) & 0xFF;
    return (int) sendto(pblk->socket, &payload, sizeof(payload), 0, (struct sockaddr *) &(pblk->serverAddress), sizeof(struct sockaddr));
}

void blk_close(PBLINKENSTATUS pblk) {
    close(pblk->socket);
    free(pblk);
}

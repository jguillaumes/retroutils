//
//  BlinkenTestClient.c
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

#include "blinkenserver.h"


int main(int argc, char **argv) {
    char *serverAddr;
    WORD serverPort;
    WORD value;
    struct protoent *udpproto;
    int udpsock = 0;
    struct sockaddr_in address;
    struct hostent *serverHost = NULL;
    PAYLOAD payload;
    
    memset(&address, 0, sizeof(address));
    memset(&payload, 0, sizeof(payload));
    
    if (argc < 3) {
        fprintf(stderr, "usage: BlinkenRestClient server_addr server_port leds_value");
        exit(1);
    }
    
    serverAddr = argv[1];
    serverPort = atoi(argv[2]);
    value = atoi(argv[3]);
    
    
    serverHost = gethostbyname(serverAddr);
    if (serverHost == NULL) {
        fprintf(stderr, "Host %s not found\n", serverAddr);
        exit(2);
    }
    
    
    udpproto = getprotobyname("udp");
    if (udpproto != NULL) {
        udpsock = socket(PF_INET, SOCK_DGRAM, udpproto->p_proto);
        if (udpsock != -1) {
            address.sin_family = AF_INET;
            address.sin_len = sizeof(address);
            address.sin_port = serverPort;
            address.sin_addr.s_addr = *((in_addr_t *)serverHost->h_addr_list[0]);
            payload.sequence = 1;
            payload.numbits = 16;
            payload.flags |= FLG_RESYNC;
            payload.data[0] = (value & 0xFF);
            payload.data[1] = (value>>8) & 0xFF;
            sendto(udpsock, &payload, sizeof(payload), 0, (struct sockaddr *) &address, sizeof(address));
            close(udpsock);
        }
    }
}
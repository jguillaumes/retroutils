//
//  blinkenserver.c
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

#ifdef HAS_BLINKEN
#include "wiringPi.h"
#include "wiringShift.h"
#endif


int startup(WORD numPort, int timeoutmilis) {
    int udpsock = 0;
    struct protoent *udpproto;
    struct timeval timeout;
    struct sockaddr_in address;
    
#ifdef HAS_BLINKEN
    if (wiringPiSetupSys() < 0) {
      syslog(LOG_ERR, "Error setting up wiringPi (%m)");
      return -1;
    }
    pinMode(CLOCK,OUTPUT);
    pinMode(LATCH,OUTPUT);
    pinMode(DATA,OUTPUT);

#else
#warning Compiling without real Blinken Lights...
#endif
    
    udpproto = getprotobyname("udp");
    if (udpproto != NULL) {
        udpsock = socket(PF_INET, SOCK_DGRAM, udpproto->p_proto);
        if (udpsock != -1) {
            timeout.tv_sec  = timeoutmilis / 1000;
            timeout.tv_usec = (timeoutmilis % 1000) * 1000;
            if (setsockopt(udpsock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != -1) {
                address.sin_family = AF_INET;
                address.sin_port = numPort;
                address.sin_addr.s_addr = INADDR_ANY;
#ifdef OSX
                address.sin_len = sizeof(address);
#endif

                if (bind(udpsock, (struct sockaddr *) &address, sizeof(address)) != -1) {
                    return udpsock;
                } else {
                    syslog(LOG_ERR, "Can't bind the UDP socket to port %d (%m)", numPort);
                    close(udpsock);
                }
            } else {
                syslog(LOG_ERR, "Can't set the UPD socket options (%m)");
                close(udpsock);
            }
        } else {
            syslog(LOG_ERR, "Can't get UDP socket (%m)");
        }
    } else {
        syslog(LOG_ALERT, "Can't get information for the udp protocol! (%m)");
    }
    return -1;
}

int getPacket(int socket, PPAYLOAD payload, PMYSTATE state) {
    size_t siz=0;
    struct sockaddr_in inaddr;
    socklen_t  addrSize = sizeof(inaddr);
    int paySize = sizeof(PAYLOAD);
    char ipaddress[16];
    
    siz = recvfrom(socket, payload, paySize, MSG_WAITALL, (struct sockaddr *) &inaddr, &addrSize);
    if (siz == -1) {
        if (errno == EAGAIN) {
            if (state->flags & FLG_BOUND) {
                syslog(LOG_NOTICE, "Timeout waiting for packets, unbinding");
                state->flags &= ~FLG_BOUND;
                memset(&(state->partner), 0, sizeof(struct sockaddr));
            }
        } else {
            syslog(LOG_ERR,"Error reading packet (%m)");
        }
        return -1;
    } else {
        state->received++;
        if (inaddr.sin_addr.s_addr == state->partner.sin_addr.s_addr) {
            dotip(inaddr.sin_addr.s_addr, ipaddress, sizeof(ipaddress));
#ifdef DEBUG
            syslog(LOG_DEBUG, "Got packet from %s", ipaddress);
#endif
            return 0;
        } else {
            if (state->flags && FLG_BOUND) {
                dotip(inaddr.sin_addr.s_addr, ipaddress, sizeof(ipaddress));
                syslog(LOG_WARNING, "Got packet from unbound address %s, ignored", ipaddress);
                state->notBound++;
                return -1;
            } else {
                dotip(inaddr.sin_addr.s_addr, ipaddress, sizeof(ipaddress));
                syslog(LOG_NOTICE, "Got packet from new client %s, bound", ipaddress);
                state->flags |= FLG_BOUND;
                memcpy(&(state->partner), &inaddr, sizeof(inaddr));
                return 0;
            }
        }
    }
}

void setBlinken(WORD bits, BYTE *bytes) {
    int i=0;
    WORD nbits = (bits <= 16 && bits > 0 ? bits : 16);
    int nbytes =    nbits / 8;
    int restbytes = nbits % 8;
    
#ifdef HAS_BLINKEN
    int lastbyte = 0;
    int b=0;
    
    digitalWrite(LATCH,LOW);
    for(i=0; i<nbytes; i++) {
        shiftOut(DATA, CLOCK, LSBFIRST, bytes[i]);
    }
    if (restbytes > 0) {
        lastbyte = bytes[nbytes];
        for(i=0; i<restbytes; i++) {
            b = (lastbyte & 0x80) >> 7;
            lastbyte <<= 1;
            digitalWrite(DATA, b);
            digitalWrite(CLOCK, HIGH);
            digitalWrite(CLOCK, LOW);
            delayMicroseconds(1);
        }
    }
    digitalWrite(LATCH,HIGH);
    
#else
    char binbuf[9];
    char linelog[256];
    int endstr = 0;
    
    memset(linelog, 0 , sizeof(linelog));
    sprintf(linelog, "Got lights packet for %d lights ", bits);
    for(i=0;i<nbytes;i++) {
        dobinary(bytes[i], binbuf, sizeof(binbuf));
        strcat(linelog, binbuf);
    }
    if (restbytes>0) {
        endstr = (int) strlen(linelog);
        dobinary(bytes[nbytes], binbuf, sizeof(binbuf));
        for (i=0; i<restbytes; i++) {
            linelog[endstr+i] = binbuf[i];
        }
    }
    syslog(LOG_DEBUG, "%s", linelog);
    printf("%s\n", linelog);
#endif
}


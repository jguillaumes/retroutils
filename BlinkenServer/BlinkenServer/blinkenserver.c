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
// #include "wiringShift.h"
#endif

static unsigned char oldBits[32];

int parity(unsigned short n) {
    unsigned short w;
    int i=0, ones=0;
    
    w = n;
    for (i=0; i<16; i++) {
        ones += w % 2;
        w /= 2;
    }
    return ones % 2;
}


int startup(WORD numPort, int timeoutmilis) {
    int udpsock = 0;
    struct protoent *udpproto;
    struct timeval timeout;
    struct sockaddr_in address;
    
    memset(oldBits, 0, sizeof(oldBits));
    
#ifdef HAS_BLINKEN
    if (wiringPiSetupSys() < 0) {
        syslog(LOG_ERR, "Error setting up wiringPi (%m)");
        return -1;
    }

    pinMode(LATCH,OUTPUT);
    
    if (wiringPiSPISetup(SPIBUS,SPISPEED) < 0) {
      syslog(LOG_ERR,"Error setting up SPI bus (%m)");
      return -1;
    }
    
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
                address.sin_port = htons(numPort);
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

int getPacket(int socket, PBLKPACKET packet, PMYSTATE state) {
    size_t siz=0;
    struct sockaddr_in inaddr;
    socklen_t  addrSize = sizeof(inaddr);
    int packSize = sizeof(BLKPACKET);
    char ipaddress[16];
    
    siz = recvfrom(socket, packet, packSize, MSG_WAITALL, (struct sockaddr *) &inaddr, &addrSize);
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
        packet->flags    = ntohs(packet->flags);
        packet->function = ntohs(packet->function);
        packet->sequence = ntohl(packet->sequence);
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

void setBlinken(PPAYLOAD payload) {
    int i=0;
#ifdef HAS_BLINKEN
    int indicators = 0;
    int par = 0;
    const int ones=0xFF;

    payload->numDataBytes  = ntohs(payload->numDataBytes);
    payload->numAddrBytes  = ntohs(payload->numAddrBytes);
    payload->numOtherBytes = ntohs(payload->numOtherBytes);
    payload->bflags        = ntohs(payload->bflags);
    
    digitalWrite(LATCH,LOW);
    if (payload->bflags & BLF_TEST) {
        for (i=0; i<payload->numDataBytes; i++) {
	  wiringPiSPIDataRW(SPIBUS, &ones, 1);
	  // shiftOut(DATA,CLOCK,MSBFIRST, 0xFF);
        }
    } else {
        if (payload->bflags & BLF_ERROR) {
            indicators = 0x02;
        } else {
            for (i=0; i< payload->numDataBytes; i++) {
                oldBits[i] = payload->data[i];
                par += parity(payload->data[i]);
            }
            if (payload->bflags & BLF_NOPARITY) {
                indicators = 0;
            } else {
                indicators = (par % 2) & 0x0001;
            }
	    wiringPiSPIDataRW(SPIBUS, &indicators, 1);
	    wiringPiSPIDataRW(SPIBUS, oldBits, payload->numDataBytes);
	    /*
            shiftOut(DATA,CLOCK,MSBFIRST,indicators);
            for (i=0; i<payload->numDataBytes; i++) {
                shiftOut(DATA,CLOCK,MSBFIRST,oldBits[i]);
            }
	    */
        }
        digitalWrite(LATCH,HIGH);
    }
#else

    char binbuf[9];
    char linelog[256];
    int endstr = 0;
    
    memset(linelog, 0 , sizeof(linelog));
    sprintf(linelog, "Got lights packet for %d lights ", payload->numDataBytes * 8);
    for(i=0;i<payload->numDataBytes;i++) {
        dobinary(payload->data[i], binbuf, sizeof(binbuf));
        strcat(linelog, binbuf);
    }
    syslog(LOG_DEBUG, "%s", linelog);
    printf("%s\n", linelog);
#endif
}


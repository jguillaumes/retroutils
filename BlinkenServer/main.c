//
//  main.c
//  BlinkenServer
//
//  Created by Jordi Guillaumes Pons on 01/01/13.
//  Copyright (c) 2013 Jordi Guillaumes Pons. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>

#include "blinkenserver.h"

#include <sys/socket.h>
#include <netinet/in.h>


int endProcess = 0;
MYSTATE state;

void int_handler(int sig) {
    if (sig == SIGINT ) {
        syslog(LOG_INFO, "SIGINT caught, exiting...");
        fprintf(stderr, "SIGINT caught, exiting...\n");
        endProcess = 1;
    }
}

void hup_handler(int sig) {
    char ipbuff[16];
    if (sig == SIGHUP ) {
        syslog(LOG_INFO, "SIGHUP caught");
        syslog(LOG_INFO, "Current state");
        syslog(LOG_INFO, "Flags         : 0x%02x", state.flags);
        dotip(state.partner.sin_addr.s_addr, ipbuff, sizeof(ipbuff));
        syslog(LOG_INFO, "Bound partner : %s", ipbuff);
        syslog(LOG_INFO, "Last seqno    : %u", state.lastSequence);
        syslog(LOG_INFO, "Received      : %u", state.received);
        syslog(LOG_INFO, "Out-of-Seq    : %u", state.outOfSequence);
        syslog(LOG_INFO, "Unbound source: %u", state.notBound);
        syslog(LOG_INFO, "Resyncs       : %u", state.resyncs);
    }
}

int main(int argc, const char * argv[])
{
    BLKPACKET packet;
    int socket = 0;
    struct sigaction sigact;
    int nmiss = 0;
    
    memset(&packet, 0, sizeof(packet));
    memset(&state, 0, sizeof(state));
    
    initLogger();
    socket = startup(DEF_PORT,5000);
    if (socket == -1) {
        fprintf(stderr, "Error getting UDP socket, see syslog for details.\n");
        exit(8);
    }
    
    syslog(LOG_NOTICE, "Server starting");
    
    sigact.sa_handler = int_handler;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGINT, &sigact, NULL);
    sigact.sa_handler = hup_handler;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGHUP, &sigact, NULL);
    
    while(!endProcess) {
        if (getPacket(socket, &packet, &state) == 0) {
            if ((packet.sequence > state.lastSequence) || (packet.flags & FLG_RESYNC)) {
                setBlinken(&packet.payload);
                nmiss = packet.sequence - state.lastSequence;
                state.lastSequence = packet.sequence;
                if (packet.flags & FLG_RESYNC) {
                    state.resyncs++;
                }
                if (nmiss > 1) {
                    syslog(LOG_WARNING, "%d packets missing. Continuing.", nmiss-1);
                }
            } else {
                syslog(LOG_WARNING, "Packet out of sequence, last processed: %u, got %u. Packet dropped.", state.lastSequence,packet.sequence);
                state.outOfSequence++;
            }
        }
    }
    printf("Finished... \n");
    close(socket);
}

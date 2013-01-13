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
#include <netdb.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "blinkenclient.h"


PBLINKENSTATUS blk_udpOpen(char *hostname, char *portNumberStr) {
    PBLINKENSTATUS bkstat = NULL;
    struct hostent *serverHost = NULL;
    struct servent *service = NULL;
    char *pnumconv = NULL;
    long portconv = 0;
    int portnum = 0;
    int udpsock = 0;
    struct protoent *udpproto;

    if (portNumberStr == NULL) {
        portnum = BLK_DEF_PORT;
    } else {
        portconv = strtol(portNumberStr, &pnumconv, 10);
        if (*pnumconv == '\0') {
            portnum = htons(portconv);
        } else {
            service =  getservbyname(portNumberStr, "udp");
            if (service != NULL) {
                portnum = service->s_port;
            } else {
                return NULL;
            }
        }
    }
    
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
    bkstat->conntype = BLKT_UDP;
    bkstat->conn.udp_status.sequence = 0;
    bkstat->conn.udp_status.socket = udpsock;
    bkstat->conn.udp_status.serverAddress.sin_family = AF_INET;
    bkstat->conn.udp_status.serverAddress.sin_port = portnum;
    bkstat->conn.udp_status.serverAddress.sin_addr.s_addr = *((in_addr_t *)serverHost->h_addr_list[0]);
    return bkstat;
}

int blk_udpSendByte(PBLINKENSTATUS pblk, BYTE b, int resync) {
    BLKPACKET packet;
    memset(&packet, 0, sizeof(BLKPACKET));
    
    packet.sequence = htonl(++(pblk->conn.udp_status.sequence));
    packet.payload.numDataBytes = htons(1);
    packet.flags = resync ? FLG_RESYNC : 0;
    packet.payload.data[0] = b;
    return (int) sendto(pblk->conn.udp_status.socket, &packet, sizeof(packet),
                        0, (struct sockaddr *) &(pblk->conn.udp_status.serverAddress),
                        sizeof(struct sockaddr));
}

int blk_udpSendWord(PBLINKENSTATUS pblk, WORD w, int resync) {
    unsigned short theword = 0;
    unsigned char *thebytes;
    BLKPACKET packet;

    theword = htons(w);
    thebytes = (unsigned char *)&theword;

    memset(&packet, 0, sizeof(BLKPACKET));
    packet.sequence = htonl(++(pblk->conn.udp_status.sequence));
    packet.payload.numDataBytes= htons(2);
    packet.flags = resync ? FLG_RESYNC : 0;
    packet.payload.data[0] = thebytes[0];
    packet.payload.data[1] = thebytes[1];
    return (int) sendto(pblk->conn.udp_status.socket, &packet, sizeof(packet),
                        0, (struct sockaddr *) &(pblk->conn.udp_status.serverAddress),
                        sizeof(struct sockaddr));
}

int blk_udpSendError(PBLINKENSTATUS pblk, int resync) {
    BLKPACKET packet;
    memset(&packet, 0, sizeof(BLKPACKET));
    
    packet.sequence = htonl(++(pblk->conn.udp_status.sequence));
    packet.payload.numDataBytes= htons(2);
    packet.flags = resync ? FLG_RESYNC : 0;
    packet.payload.bflags = htons(BLF_ERROR);
    packet.payload.data[0] = 0;
    packet.payload.data[1] = 0;
    return (int) sendto(pblk->conn.udp_status.socket, &packet, sizeof(packet),
                        0, (struct sockaddr *) &(pblk->conn.udp_status.serverAddress),
                        sizeof(struct sockaddr));
}

void blk_udpClose(PBLINKENSTATUS pblk) {
    close(pblk->conn.udp_status.socket);
    free(pblk);
}

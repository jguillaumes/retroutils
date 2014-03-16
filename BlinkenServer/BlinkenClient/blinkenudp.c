/* blinkenudp.c: BlinkenLights for the PDP11
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


/*
 * UDP transport implementation
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

/*
 * Establish an udp transport endpoint to send the lites information
 * Parameters:
 *  hostname: 
 *            Name (or IP address in dot format) of the IP host running
 *            the blikenlights server.
 *  portNumberStr:
 *            Number (or service name in /etc/services) of the port where
 *            the blinkenlights server is listening.
 * Returns:
 *  If the process completes sucesfully, the address of a newly allocated
 *  BLINKENSTATUS structure. If it does not, it returns NULL.
 */
PBLINKENSTATUS blk_udpOpen(char *hostname, char *portNumberStr) {
    PBLINKENSTATUS bkstat = NULL;               // Status block pointer (to be allocated)
    struct hostent *serverHost = NULL;
    struct servent *service = NULL;
    char *pnumconv = NULL;
    long portconv = 0;
    int portnum = 0;
    int udpsock = 0;
    struct protoent *udpproto;

    /*
     * Find the destination port
     * If not specified, will take the default (from blinkenudp.h)
     * If specified, will try to convert to numeric. If the conversion
     * is correct, we take that numer. If it is not correct we try to
     * get the port number from /etc/services using getservbyname()
     */
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
    
    /*
     * Get host address from name 
     */
    serverHost = gethostbyname(hostname);
    if (serverHost == NULL) {
        fprintf(stderr, "Host %s not found\n", hostname);
        return NULL;
    }
    
    /*
     * Get UDP protocol number
     * */
    udpproto = getprotobyname("udp");
    if (udpproto == NULL) {
        fprintf(stderr, "Error getting information for protocol udp\n");
        fprintf(stderr, "This is probably a configuration problem!\n");
        return NULL;
    }
    
    /*
     * Get our endpoint for the UDP server
     */
    udpsock = socket(PF_INET, SOCK_DGRAM, udpproto->p_proto);
    if (udpsock < 0) {
        perror("getting socket");
        return NULL;
    }
    
    /*
     * If we reach this point we have an opened socket, so we can allocate
     * and build the status block.
     */
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

/*
 * Send a byte to the blinkenlights server using UDP
 * Parameters:
 *  pblk  : pointer to an active BLINKENSTATUS structure
 *  b     : byte to send
 *  resync: If true, activate FLG_RESYNC in the outgoing message and
 *          reset the sequence number to one
 * Returns: Result of the sendto() call
 *
 */
int blk_udpSendByte(PBLINKENSTATUS pblk, BYTE b, int resync) {
    BLKPACKET packet;                           // Data packet
    memset(&packet, 0, sizeof(BLKPACKET));
    
    packet.payload.numDataBytes = htons(1);     // All the ints sent in network order
    packet.function = htons(BLK_DATA);
    packet.flags =    resync ? htons(FLG_RESYNC) : 0;
    packet.sequence = resync ? 1 : htonl(++(pblk->conn.udp_status.sequence));
    packet.payload.data[0] = b;
    return (int) sendto(pblk->conn.udp_status.socket, &packet, sizeof(packet),
                        0, (struct sockaddr *) &(pblk->conn.udp_status.serverAddress),
                        sizeof(struct sockaddr));
}

/*
 * Send a word (16 bits integer) to the blinkenlights server using UDP
 * Parameters:
 *  pblk  : pointer to an active BLINKENSTATUS structure
 *  w     : word to send, in host order. The function sends it in
 *          network order.
 *  resync: If true, activate FLG_RESYNC in the outgoing message and
 *          reset the sequence number to one
 * Returns: Result of the sendto() call
 */
int blk_udpSendWord(PBLINKENSTATUS pblk, WORD w, int resync) {
    unsigned short theword = 0;
    unsigned char *thebytes;
    BLKPACKET packet;

    theword = htons(w);                     // Put the word in network order
    thebytes = (unsigned char *)&theword;   // Point to the start of the word bytes

    memset(&packet, 0, sizeof(BLKPACKET));
    packet.payload.numDataBytes= htons(2);  // Size, in network order
    packet.function = htons(BLK_DATA);
    packet.flags    = resync ? htons(FLG_RESYNC) : 0;
    packet.sequence = resync ? 1 : htonl(++(pblk->conn.udp_status.sequence));
    packet.payload.data[0] = thebytes[0];   // copy first byte
    packet.payload.data[1] = thebytes[1];   // copy second byte
    return (int) sendto(pblk->conn.udp_status.socket, &packet, sizeof(packet),
                        0, (struct sockaddr *) &(pblk->conn.udp_status.serverAddress),
                        sizeof(struct sockaddr));
}


int blk_udpSendError(PBLINKENSTATUS pblk, int resync) {
    BLKPACKET packet;
    memset(&packet, 0, sizeof(BLKPACKET));
    
    packet.sequence = htonl(++(pblk->conn.udp_status.sequence));
    packet.payload.numDataBytes= htons(2);
    packet.function = htons(BLK_DATA);
    packet.flags = resync ? htons(FLG_RESYNC) : 0;
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

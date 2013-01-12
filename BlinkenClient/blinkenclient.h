/* blinkenclient.h: BlinkenLights for the PDP11
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



#ifndef BlinkenServer_blinkenclient_h
#define BlinkenServer_blinkenclient_h

#include <limits.h>
#include <netinet/in.h>

#include "blinkenserial.h"
#include "blinkenudp.h"

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int LONGWORD;

#define DEF_PORT    11696

/*
 * Connection / hardware type
 */
typedef enum {
    BLKT_LOCAL = 0,
    BLKT_SERIAL = 1,
    BLKT_UDP = 2
} blnk_conntype;

/*
 *   Message (packet) flags
 */
#define FLG_RESYNC  0x0001
#define FLG_DETACH  0x0002

/*
 * Payload (bits) flags
 */
#define BLF_NOPARITY 0x0001
#define BLF_ERROR    0x0002
#define BLF_TEST     0x0004

/*
 * Function codes
 */
#define BLK_DATA     0

#pragma pack(2)
struct s_payload {
    WORD          bflags;
    WORD          numDataBytes;
    WORD          numAddrBytes;
    WORD          numOtherBytes;
    BYTE          data[32];
};

typedef struct s_payload PAYLOAD;
typedef struct s_payload *PPAYLOAD;

struct s_blkpacket {
  WORD        function;
  WORD        flags;
  LONGWORD    sequence;
  struct s_payload payload;
};

typedef struct s_blkpacket BLKPACKET;
typedef struct s_blkpacket *PBLKPACKET;

struct s_blinkenstatus {
    blnk_conntype conntype;
    union {
        UDPSTATUS udp_status;
        TTYSTATUS tty_status;
    } conn;
};

typedef struct s_blinkenstatus BLINKENSTATUS;
typedef struct s_blinkenstatus *PBLINKENSTATUS;
#pragma pack()

/*
 * Function prototypes
 */
PBLINKENSTATUS blk_open(char *connString);
int blk_sendByte(PBLINKENSTATUS pblk, BYTE b, int resync);
int blk_sendWord(PBLINKENSTATUS pblk, WORD w, int resync);
int blk_sendError(PBLINKENSTATUS pblk, int resync);
void blk_close(PBLINKENSTATUS pblk);

PBLINKENSTATUS blk_udpOpen(char *server, char *port);
int blk_udpSendByte(PBLINKENSTATUS pblk, BYTE b, int resync);
int blk_udpSendWord(PBLINKENSTATUS pblk, WORD w, int resync);
int blk_udpSendError(PBLINKENSTATUS pblk, int resync);
void blk_udpClose(PBLINKENSTATUS pblk);

PBLINKENSTATUS blk_serialOpen(char *server);
int blk_serialSendByte(PBLINKENSTATUS pblk, BYTE b, int resync);
int blk_serialSendWord(PBLINKENSTATUS pblk, WORD w, int resync);
int blk_serialSendError(PBLINKENSTATUS pblk);
void blk_serialClose(PBLINKENSTATUS pblk);


#endif

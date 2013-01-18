//
//  blinkenserver.h
//  BlinkenServer
//
//  Created by Jordi Guillaumes Pons on 01/01/13.
//  Copyright (c) 2013 Jordi Guillaumes Pons. All rights reserved.
//

#ifndef BlinkenServer_blinkenserver_h
#define BlinkenServer_blinkenserver_h

#include <netinet/in.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int LONGWORD;

#ifdef HAS_BLINKEN
#define SPIBUS   0
#define LATCH    8
#define SPISPEED 1000000
#endif


#define FLG_RESYNC  0x01
#define FLG_DETACH  0x02

#define BLF_NOPARITY 0x0001
#define BLF_ERROR    0x0002
#define BLF_TEST     0x0004

#define DEF_PORT    11696

#pragma pack(2)
struct s_payload {
    WORD          bflags;
    WORD          numDataBytes;
    WORD          numAddrBytes;
    WORD          numOtherBytes;
    BYTE          data[16];
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

#define FLG_BOUND       0x01
#define FLG_HASDROPS    0x02

struct s_mystate {
    BYTE   flags;
    struct sockaddr_in  partner;
    LONGWORD    lastSequence;
    LONGWORD    received;
    LONGWORD    outOfSequence;
    LONGWORD    notBound;
    LONGWORD    resyncs;
};

typedef struct s_mystate MYSTATE;
typedef struct s_mystate *PMYSTATE;

int startup(WORD numPort, int timeoutMilis);
void initLogger();
int getPacket(int socket, PBLKPACKET packet, PMYSTATE state);
void setBlinken(PPAYLOAD payload);
int dotip(int address, char *buffer, int bufsiz);
int dobinary(BYTE byte, char *buffer, int bufsiz);

#endif


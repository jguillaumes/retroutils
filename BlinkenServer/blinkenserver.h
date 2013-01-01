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
#define CLOCK 17
#define LATCH 18
#define DATA  27
#endif


#define FLG_RESYNC  0x01
#define FLG_DETACH  0x02

#define DEF_PORT    11696


struct s_payload {
    BYTE        function;
    BYTE        flags;
    LONGWORD    sequence;
    WORD        numbits;
    BYTE        data[32];
};

typedef struct s_payload PAYLOAD;
typedef struct s_payload *PPAYLOAD;

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
int getPacket(int socket, PPAYLOAD payload, PMYSTATE state);
void setBlinken(WORD bits, BYTE *bytes);
int dotip(int address, char *buffer, int bufsiz);
int dobinary(BYTE byte, char *buffer, int bufsiz);

#endif


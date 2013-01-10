//
//  blinkenclient.h
//  BlinkenServer
//
//  Created by Jordi Guillaumes Pons on 01/01/13.
//  Copyright (c) 2013 Jordi Guillaumes Pons. All rights reserved.
//

#ifndef BlinkenServer_blinkenclient_h
#define BlinkenServer_blinkenclient_h

#include <netinet/in.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int LONGWORD;

#define FLG_RESYNC  0x01
#define FLG_DETACH  0x02

#define DEF_PORT    11696

#define BLF_NOPARITY 0x0001
#define BLF_ERROR    0x0002
#define BLF_TEST     0x0004

#define DEF_PORT    11696

#pragma pack(1)

struct s_payload {
  WORD          bflags;
  WORD          numBytes;
  BYTE          data[32];
};

typedef struct s_payload PAYLOAD;
typedef struct s_payload *PPAYLOAD;

struct s_blkpacket {
  BYTE        function;
  BYTE        flags;
  LONGWORD    sequence;
  struct s_payload payload;
};

typedef struct s_blkpacket BLKPACKET;
typedef struct s_blkpacket *PBLKPACKET;

struct s_blinkenstatus {
    struct      sockaddr_in serverAddress;
    int         socket;
    LONGWORD    sequence;
};

typedef struct s_blinkenstatus BLINKENSTATUS;
typedef struct s_blinkenstatus *PBLINKENSTATUS;

#pragma pack()

PBLINKENSTATUS blk_setup(char *hostname, WORD portNumber);
int blk_sendbyte(PBLINKENSTATUS pblk, BYTE b, int resync);
int blk_sendword(PBLINKENSTATUS pblk, WORD w, int resync);
int blk_senderror(PBLINKENSTATUS pblk, int resync);
void blk_close(PBLINKENSTATUS pblk);


#endif

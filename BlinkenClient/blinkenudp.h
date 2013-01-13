//
//  blinkenudp.j.h
//  BlinkenServer
//
//  Created by Jordi Guillaumes Pons on 12/01/13.
//  Copyright (c) 2013 Jordi Guillaumes Pons. All rights reserved.
//

#ifndef BlinkenServer_blinkenudp_h
#define BlinkenServer_blinkenudp_h

#define BLK_DEF_PORT    11696


struct udp_status_s {
    struct  sockaddr_in serverAddress;
    int     socket;
    unsigned long sequence;
};

typedef struct udp_status_s UDPSTATUS;
typedef struct udp_status_s *PUDPSTATUS;

#endif

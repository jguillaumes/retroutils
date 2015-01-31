/* 
 * File:   DecnetDefs.h
 * Author: jguillaumes
 *
 * Definitions (constants, types and structures) to handle DECNET packets
 * 
 * Created on 28 / gener / 2015, 23:29
 */

#ifndef DECNETDEFS_H
#define	DECNETDEFS_H

/*
 * Macros to extract area and node from a decnet address word
 */
#define NODE(addr)  ((addr)&0b1111111111)
#define AREA(addr)  ((addr)>>10)

/*
 * Offsets into the ethernet frame
 */
#define OFS_ETHERTYPE       12
#define OFS_FRAMESIZE       14
#define OFS_FRAME           16

/*
 * Ethertype words
 */
#define ET_DNETROUTING      0x0360
#define ET_VLANTAG          0x0081

/*
 * Useful typedefs
 */
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int LONGWORD;
typedef BYTE ETHADDR[6];
typedef WORD DECADDR;


#pragma pack(1)

/*
 * Routing flags
 */
struct routing_flags_s {
    unsigned int ctype : 1;
    unsigned int type : 3;
    unsigned int filler : 3;
    unsigned int padding : 1;
};

/*
 * Node type and message flags
 */
struct node_flags_s {
    unsigned int nodeType : 2;
    unsigned int verificationRequired : 1;
    unsigned int rejectFlag : 1;
    unsigned int verificationFailed : 1;
    unsigned int noMulticast : 1;
    unsigned int blockingRequest : 1;
    unsigned int : 1;
};

/*
 * Init message
 */
struct init_t {
    struct routing_flags_s routingFlags;
    DECADDR srcNode;
    struct node_flags_s nodeInfo;
    WORD blkSize;
    BYTE version[3];
    WORD timer;
};

/*
 * Hello message
 */
struct hello_t {
    struct routing_flags_s routingFlags;
    BYTE version[3];
    BYTE filler[4];
    DECADDR dnAddr;
    struct node_flags_s nodeInfo;
    WORD blkSize;

    union {

        struct __attribute__((packed)) {
            BYTE area;
            BYTE seed[8];
            ETHADDR designatedRouter;
            WORD helloTimer;
            BYTE reserved;
            BYTE data[0];
        }
        endNode;

        struct __attribute__((packed)) {
            BYTE priority;
            BYTE area;
            WORD helloTimer;
            BYTE reserved;

            struct __attribute__((packed)) {
                ETHADDR router;
                BYTE priState;
            }
            eList[0];
        }
        router;
    } u;
};

/*
 *  Ethernet frame
 */
struct frame_s {
    ETHADDR src;
    ETHADDR dst;

    union {

        struct {
            WORD etherType;
            WORD length;
            BYTE payload[0];
        } nonTagged;

        struct {
            WORD vlanType;
            WORD etherType;
            WORD length;
            BYTE payload[0];
        } tagged;
    } u;
};

#pragma pack()

#endif	/* DECNETDEFS_H */


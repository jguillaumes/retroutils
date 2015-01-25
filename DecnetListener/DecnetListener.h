/* 
 * File:   DecnetListener.h
 * Author: jguillaumes
 *
 * Created on 24 / gener / 2015, 20:39
 */

#ifndef DECNETLISTENER_H
#define	DECNETLISTENER_H

#ifdef	__cplusplus
extern "C" {
#endif

#define NODE(addr)  ((addr)&0b1111111111)
#define AREA(addr)  ((addr)>>10)

#define OFS_ETHERTYPE       12
#define OFS_FRAMESIZE       2
#define OFS_FRAME           2

#define ET_DNETROUTING      0x0360
#define ET_VLANTAG          0x0081

    /*
     * Useful typdefs
     */
    typedef unsigned char BYTE;
    typedef unsigned short WORD;
    typedef unsigned int LONGWORD;
    typedef BYTE MACADDR[6];
    typedef WORD DNADDR;

    /*
     * Routing flags
     */
    struct __attribute__((packed)) routing_flags_s {
        unsigned int ctype :1;
        unsigned int type :3;
        unsigned int filler :3;
        unsigned int padding :1;
    };

    /*
     * Node type and message flags
     */
    struct __attribute__((packed)) node_flags_s {
        unsigned int nodeType :2;
        unsigned int verificationRequired :1;
        unsigned int rejectFlag :1;
        unsigned int verificationFailed :1;
        unsigned int noMulticast :1;
        unsigned int blockingRequest :1;
        unsigned int :1;
    };
    
    /*
     * Init message
     */
    struct __attribute__((packed)) init_t {
        struct routing_flags_s routingFlags;
        DNADDR srcNode;
        struct node_flags_s nodeInfo;
        WORD blkSize;
        BYTE version[3];
        WORD timer;
    };
    
    /*
     * Hello message
     */
    struct __attribute__((packed)) hello_t {
        struct routing_flags_s routingFlags;
        BYTE version[3];
        BYTE filler[4];
        DNADDR dnAddr;
        struct node_flags_s nodeInfo;
        WORD blkSize;
        BYTE area;
        BYTE seed[8];
        MACADDR designatedRouter;
        WORD helloTimer;
        BYTE reserved;
        BYTE data[1];
    };

    /*
     *  Ethernet frame
     */ 
    struct __attribute__((packed)) frame_s {
        MACADDR src;
        MACADDR dst;
        union {
            struct {
                WORD etherType;
                WORD length;
                struct hello_t hello;
            } nonTagged;
            struct {
                WORD vlanType;
                WORD etherType;
                WORD length;
                struct hello_t hello;
            } tagged;
        } u;
    };

    /*
     * Forward declarations
     */
    void processHello(struct hello_t *hello);
    void processInit(struct init_t *hello);
    WORD dnFromMac(BYTE *mac);
    void typeName(unsigned int nodeType, char *type);
    FILE *openPacketFile(char *filename);
    void writePacket(FILE *file, long len, BYTE *packet);

#ifdef	__cplusplus
}
#endif

#endif	/* DECNETLISTENER_H */

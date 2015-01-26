// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _DecnetDisplay_H_
#define _DecnetDisplay_H_
#include <Arduino.h>
//add your includes for the project DecnetListener here
#include <UTFT.h>


//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif

#define MYAREA      7
#define FONTWIDTH	16
#define FONTHEIGHT	16
#define PANWIDTH	318
#define PANHEIGHT	198
#define LINES		(PANHEIGHT/(FONTHEIGHT+2))
#define COLUMNS		(PANWIDTH/(6*FONTWIDTH+2))

#define LED_PIN		13
#define CYCLE_MILLIS 60000
#define SECOND_MILLIS 1000
#define CHECK_MILLIS 100

#define BKG_STD		VGA_BLACK
#define FG_STD		VGA_WHITE
#define BKG_OFFLINE	VGA_BLACK
#define FG_OFFLINE	VGA_GRAY
#define BKG_NEW		VGA_BLUE
#define FG_NEW		VGA_WHITE
#define BKG_LOST	VGA_MAROON
#define FG_LOST		VGA_RED
#define BKG_ROUTER	VGA_GREEN
#define FG_ROUTER	VGA_LIME
#define BKG_ROUTER2	VGA_NAVY
#define FG_ROUTER2	VGA_AQUA

enum nodeStatus_e { OFFLINE, HELLO, ENDNODE, ROUTER, ROUTER2, LOST };

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
    typedef unsigned long LONGWORD;
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
        MACADDR dst;
        MACADDR src;
        union {
            struct {
                WORD etherType;
                WORD length;
                BYTE payload[1];
            } nonTagged;
            struct {
                WORD vlanType;
                WORD etherType;
                WORD length;
                BYTE payload[1];
            } tagged;
        } u;
    };



struct node_s {
	unsigned int dnaddr;
	char name[7];
	long countdown;
	enum nodeStatus_e status;
	int dpyX;
	int dpyY;
};

//add your function definitions for the project DecnetListener here

void dumpPacket(int offset, int len);
void printHexByte(int b);
void analyzePacket(uint16_t offset, uint16_t len);
int getDecnetAddress(byte *macPtr);
void getDecnetName(unsigned int addr, char *buffer);
void displayString(UTFT &lcd, int col, int fila, char *string, int background, int color);
struct node_s *dicotomica(unsigned int addr, int inici, int fi);
void displayNode(UTFT &lcd, struct node_s &node);
void displayClock(unsigned long millis);

//Do not add code below this line
#endif /* _DecnetDisplay_H_ */

// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _DecnetDisplay_H_
#define _DecnetDisplay_H_
#include <Arduino.h>
//add your includes for the project DecnetListener here


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
#define FONTWIDTH	12
#define FONTHEIGHT	12
#define PANWIDTH	158
#define PANHEIGHT	100
#define LINES		(PANHEIGHT/(FONTHEIGHT+2))
#define COLUMNS		(PANWIDTH/(6*FONTWIDTH+2))

#define LED_PIN		13
#define CYCLE_MILLIS 60000
#define SECOND_MILLIS 1000
#define CHECK_MILLIS 100

const int BKG_STD[3] = 		{0,0,0};
const int FG_STD[3] = 		{255,255,255};
const int BKG_OFFLINE[3] = 	{0,0,0};
const int FG_OFFLINE[3] = 	{192,192,192};
const int BKG_NEW[3] = 		{0,0,204};
const int FG_NEW[3] = 		{255,255,255};
const int BKG_LOST[3] = 	{102,51,0};
const int FG_LOST[3] = 		{204,0,0};
const int BKG_ROUTER[3] = 	{0,204,0};
const int FG_ROUTER[3] = 	{128,255,0};
const int BKG_ROUTER2[3] = 	{0,0,102};
const int FG_ROUTER2[3] = 	{0,102,204};
const int VGA_RED[3] = 		{204,0,0};
const int VGA_BLACK[3] = 	{0,0,0};

#define BIG_FONT	ucg_font_courR10
#define SMALL_FONT	ucg_font_courR08

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
void displayString(int col, int fila, char *string, int *background, int *color);
struct node_s *dicotomica(unsigned int addr, int inici, int fi);
void displayNode(struct node_s &node);
void displayClock(unsigned long millis);

//Do not add code below this line
#endif /* _DecnetDisplay_H_ */

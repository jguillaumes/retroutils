////////////////////////////////////////////////////////////////////////////
// DECNET Hello listener V01.00                                           //
////////////////////////////////////////////////////////////////////////////

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

// #define DEBUG 1

/*
 * Macros to extract area and node from a decnet address word
 */
#define NODE(addr)  ((addr)&0b1111111111)
#define AREA(addr)  ((addr)>>10)

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

// Customization and TFT parameters
// Increment MAX_NODES at your own risk!
#define MAX_NODES      24
#define FONTWIDTH	8
#define FONTHEIGHT	10
#define PANWIDTH	158
#define PANHEIGHT	118
#define LINES		(PANHEIGHT/(FONTHEIGHT+2))
#define COLUMNS		(PANWIDTH/(6*FONTWIDTH+2))
#define SECOND_MILLIS 1000
#define CHECK_MILLIS 100

// PIN definitions.
// BL_PIN must be a PWM capable one
// ETH_CS, SD_CS and TFT_CS set the CS line for
// the ethernet, the SD and the TFT respectively
// The rest of the lines must be connected to the
// hardware SPI pins
#define BL_PIN     3
#define ETH_CS     7
#define SD_CS      6
#define TFT_CS    10


// Color definitions (RGB)
const int BKG_STD[3] = 		{0,0,0};
const int FG_STD[3] = 		{250,250,250};
const int BKG_OFFLINE[3] = 	{0,0,0};
const int FG_OFFLINE[3] = 	{64,64,64};
const int BKG_NEW[3] = 		{0,0,102};
const int FG_NEW[3] = 		{255,255,255};
const int BKG_LOST[3] = 	{128,0,0};
const int FG_LOST[3] = 		{255,228,225};
const int BKG_ROUTER[3] = 	{0,100,0};
const int FG_ROUTER[3] = 	{202,255,0};
const int BKG_ROUTER2[3] = 	{25,25,112};
const int FG_ROUTER2[3] = 	{152,245,255};
const int VGA_RED[3] = 		{204,0,0};
const int VGA_BLACK[3] = 	{0,0,0};

// DECNET related stuff
// Multiplier for adjacency timer - Architectural constant
#define BCT3MULT  3    

enum nodeStatus_e { OFFLINE=0, HELLO=1, ENDNODE=2, ROUTER=3, ROUTER2=4, LOST=5 };
#define NODE(addr)  ((addr)&0b1111111111)
#define AREA(addr)  ((addr)>>10)

#define ET_DNETROUTING      0x0360
#define ET_VLANTAG          0x0081

// Error messages. 24 bytes max!!
//                             ....+....1....+....2.... 
const char sERR00[] PROGMEM = "ERR: Card";        // 0
const char sERR01[] PROGMEM = "ERR: EThernet";    // 1
const char sERR02[] PROGMEM = "ERR: Nodefile";    // 2
const char sERR03[] PROGMEM = "ERR: Alloc";       // 3
const char sERR04[] PROGMEM = "ERR: no nodelist"; // 4
const char sERR05[] PROGMEM = "ERR: EEPROM eye";  // 5

//
// Message strings. 24 bytes max!!!
//                                ....+....1....+....2.... 
const char sDNETLSN[] PROGMEM = { "DECNET Listener" };
const char sINITLCD[] PROGMEM = { "Init LCD" };
const char sINITETH[] PROGMEM = { "Init ETH" };
const char sINITSD[]  PROGMEM = { "Init SD" };
const char sOPENNF[]  PROGMEM = { "Open nodefile" };
const char sOPENOK[]  PROGMEM = { "Open OK" };
const char sREADFIL[] PROGMEM = { "Read nodefile" };
const char sREADOK[]  PROGMEM = { "Read OK" };
const char sRDEPROM[] PROGMEM = { "Read EEPROM" };
const char sWREPROM[] PROGMEM = { "Write EEPROM" };
const char sOKEPROM[] PROGMEM = { "EEPROM OK" };
const char sREADY[]   PROGMEM = { "Ready!" };

const char* const msg_table[] PROGMEM = { sDNETLSN, sINITLCD, sINITETH, sINITSD,  //  0 -  3
                                          sOPENNF,  sOPENOK,  sREADFIL, sREADOK,  //  4 -  7
                                          sRDEPROM, sWREPROM, sOKEPROM, sREADY,   //  8 - 11
                                          sERR00,   sERR01,   sERR02,   sERR03,   // 12 - 15
                                          sERR04,   sERR05                        // 16 - 17 
                                      }; 

#define DNETLSN  0
#define INITLCD  1
#define INITETH  2
#define INITSD   3
#define OPENNF   4
#define OPENOK   5
#define READFIL  6
#define READOK   7
#define RDEPROM  8
#define WREPROM  9
#define OKEPROM 10
#define READY   11
#define ERR00   12
#define ERR01   13
#define ERR02   14
#define ERR03   15
#define ERR04   16
#define ERR05   17

// 
// Structures to map the ethernet packets
//
/*                                        
 * Routing flags
 */
struct __attribute__((packed)) routing_flags_s {
    unsigned int ctype : 1;
    unsigned int type : 3;
    unsigned int filler : 3;
    unsigned int padding : 1;
};

/*
 * Node type and message flags
 */
struct __attribute__((packed)) node_flags_s {
    unsigned int nodeType : 2;
    unsigned int verificationRequired : 1;
    unsigned int rejectFlag : 1;
    unsigned int verificationFailed : 1;
    unsigned int noMulticast : 1;
    unsigned int blockingRequest : 1;
    unsigned int : 1;
};


/*
 * Hello message
 */
struct __attribute__((packed)) hello_t {
    struct routing_flags_s routingFlags;
    BYTE version[3];
    BYTE filler[4];
    DECADDR dnAddr;
    struct node_flags_s nodeInfo;
    WORD blkSize;

    union {
        // Hello for EndNodes
        struct __attribute__((packed)) {
            BYTE area;
            BYTE seed[8];
            ETHADDR designatedRouter;
            WORD helloTimer;
            BYTE reserved;
            BYTE data[0];
        }
        endNode;
        // Hello for routers
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
struct __attribute__((packed)) frame_s {
    ETHADDR dst;
    ETHADDR src;

    union {
        // Non-tagged (no VLAN)
        struct __attribute__((packed)) {
            WORD etherType;
            WORD length;
            BYTE payload[0];
        } nonTagged;
        // Tagged (VLAN)
        struct __attribute__((packed)) {
            WORD vlanType;
            WORD etherType;
            WORD length;
            BYTE payload[0];
        } tagged;
    } u;
};

/*
* Node database
*/
struct __attribute__((packed)) node_s  {
	unsigned int dnaddr;
	char name[6];
	int htimer;                 // Hello timer (T3) in seconds
        long countdown;              // Countdown in millis
	enum nodeStatus_e status;
        BYTE nrow;
        BYTE ncol;
};

//add your function definitions for the project DecnetListener here

#ifdef DEBUG
void dumpPacket(int offset, int len);
void printHexByte(int b);
#endif

void analyzePacket(uint16_t offset, uint16_t len);
int getDecnetAddress(byte *macPtr);
void getDecnetName(unsigned int addr, char *er);
void displayString(int col, int fila, char *string, int *background, int *color);
struct node_s *dicotomica(unsigned int addr, int inici, int fi);
void displayNode(struct node_s &node);
void displayClock(unsigned long millis);
void fatal(int);
bool loadFile(int *numNodes, struct node_s *nodes);
void serialmem(int);
//Do not add code below this line
#endif /* _DecnetDisplay_H_ */


/////////////////////////////////////////////////////////////////////////////
// DECNET Hello listener V01.00                                            //
/////////////////////////////////////////////////////////////////////////////
// Listen to broadcasted DECNET routing packets and
// display the status of certain nodes in a TFT screen
//
// This sketch requires:
// - ENJ2860 based etherned interface
// - SPI TFT, based on the ST7735 chip
// - SD reader card (with SPI bus)
//
// I run it on a standalone ATMEGA328P chip hooked into a
// contraption I made for it. The sketch uses about 25K of
// flash and almost all the SRAM. 
//
// The TFT I use inverts red and blue, and it is taken
// into account in the sketch. If yours is a true RGB, you'll need
// to make some changes
//
// Libraries used:
// - TFT, SPI and EEPROM (included in the Arduino IDE > 1.5)
// - SdFat (https://github.com/greiman/SdFat)
// - Ethercard (for the ENJ2860 https://github.com/jcw/ethercard)
/////////////////////////////////////////////////////////////////////////////
// Usage:
// To load the nodelist, you must use a FAT formatted SDCARD with a NODES.DAT 
// file in its root directory.
// The first line of the file must contain the total number of nodes. The
// current maximum is 24 (perhaps this number could be increased a little bit,
// but not a lot).
// The following lines must contain a decnet address (in a.N format) followed by
// the node name.
// Example:
// --- BEGIN ---
// 2
// 7.1,MYNOD1
// 7.2,MYNOD2
// --- END ---
//
// the BEGIN and END lines MUST NOT BE in the file, they are just markers for this
// example.
// The file MUST be ordered by DECNET address.
// The format and the order ARE NOT VERIFIED ON LOAD. 
//
// The sketch will load the nodetable from the file and then will save it into
// the 328p EEPROM space. At each boot it will try to find first an SD card with
// the nodelist, and if there is not one present, it wull try to load it from
// EEPROM. So you need to insert a card just when you modify the nodelist.
//
// WARNING: There is an issue which causes the EEPROM value not to be correctly
// saved every time. Sometimes the contents of the EEPROM are incorrect. 
/////////////////////////////////////////////////////////////////////////////

#include <ArduinoStream.h>
#include <bufstream.h>
#include <ios.h>
#include <iostream.h>
#include <istream.h>
#include <MinimumSerial.h>
#include <ostream.h>
#include <Sd2Card.h>
#include <SdBaseFile.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <SdFatmainpage.h>
#include <SdFatUtil.h>
#include <SdFile.h>
#include <SdInfo.h>
#include <SdSpi.h>
#include <SdStream.h>
#include <SdVolume.h>
#include <StdioStream.h>
#include <avr/pgmspace.h>

#include <TFT.h>
#include <EtherCard.h>
#include <enc28j60.h>
#include <SPI.h>
#include <EEPROM.h>

#include "DecnetTFT.h"
#include "LCDConsole.h"

TFT screen = TFT(10, 9, 8);
LCDConsole cons;

// DECNET Multicast Addresses
static const byte mCastHelloEN[] = { 0xAB, 0x00, 0x00, 0x03, 0x00, 0x00 };
static const byte mCastHelloRT[] = { 0xAB, 0x00, 0x00, 0x04, 0x00, 0x00 };
static const byte mCastL2RT[]    = { 0x09, 0x00, 0x2b, 0x02, 0x00, 0x00 };

// Our own Mac Address (will not be broadcasted)
static const byte dnMac[]        = { 0xaa, 0x00, 0x04, 0x00, 0xe7, 0x1f };

// Watched node table. 
static struct node_s nodes[MAX_NODES];
int numNodes;

// Etheret adapter stuff
ENC28J60 card;
byte ENC28J60::buffer[128];


// Timekeeping
unsigned long milliseconds = 0;
unsigned long lastMilliseconds = 0;
unsigned long secondControl = 0;


//+
// Setup the environment
// - load the list of nodes
// - prepare the screen
// - initialize the ethernet device
//-
void setup() {
  int i;

  // Turn on TFT backlight
  pinMode(BL_PIN, OUTPUT);
  analogWrite(BL_PIN, 200);
  
  // Initialize the screen and say hello
  screen.begin();
  cons.begin(screen);
  cons.printmem(DNETLSN);

  // load the node list. See comments in SDFile.cpp
  loadFile(&numNodes, nodes);

  // Prepare the screen (and from now on, use the UART
  // to log stuff).
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  cons.printmem(INITLCD);
  Serial.begin(9600);
  screen.background(0, 0, 0);
  screen.stroke(0, 204, 0);
  screen.noFill();
  screen.rect(0, 0, 159, 117);

  // Initialize the ethernet device
  serialmem(INITETH);
  card.initSPI();
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  if (card.initialize(sizeof Ethernet::buffer, dnMac, ETH_CS) == 0) {
    fatal(ERR01);
  } else {
    card.enableBroadcast();
    card.enableMulticast();
  }
  
  // Make know we are set up and ready
  serialmem(READY);

  // Display the list of nodes
#ifdef DEBUG
  Serial.println("Nodes---");
  for (i = 0; i < numNodes; i++) {
    Serial.print(nodes[i].dnaddr);
    Serial.print(": ");
    Serial.println(nodes[i].name);
    displayNode(&nodes[i]);
  }
  Serial.println("End---.");
#else
  for (i = 0; i < numNodes; i++) {
    displayNode(&nodes[i]);
  }
#endif 

  // Prepare the clock
  screen.stroke(0, 225, 224); // Yellow (B-G-R)
  screen.text("Up:", 0, 118);
  lastMilliseconds = millis();
}


//+
// Main loop. 
// - Get a ethernet frame and analyze it
// - Check the timekeeping
//   * Each second, update the clock, 
//   * Each CHECK_MILLIS, verify the DECNET nodes are still alive
//   * Update the changed nodes in screen
//-
void loop() {

  int i;
  uint16_t len;
  long interval;

  len = card.packetReceive();
  if (len > 0) analyzePacket(0, len);

  milliseconds = millis();
  
  // Clock update, each second
  if (milliseconds - secondControl >= SECOND_MILLIS) {
    secondControl = milliseconds;
    displayClock(secondControl);
  }

  // Node verification, each CHECK_MILLIS
  interval = (int) (milliseconds - lastMilliseconds);
  if (interval >= CHECK_MILLIS) {
    lastMilliseconds = milliseconds;

    // For each node:
    // - Subtract the interval since last check from the countdown
    // - If the countdown has reached zero, set down the adjacency status
    //   (two steps: ONLINE -> LOST -> OFFLINE)
    //   For LOST status we will wait again for BCT3MUKT hello periods before
    //   marking the node OFFLINE.
    // - If the status has changed (either to LOST or to OFFLINE),
    //   redisplay the node
    for (i = 0; i < numNodes; i++) {
      if (nodes[i].status != OFFLINE) {
        nodes[i].countdown -= interval;
#ifdef DEBUG
        char msgbuff[40];
        sprintf(msgbuff, "Node: %d (%s), ht=%ld cd=%ld, intv=%ld", i, nodes[i].name,
                nodes[i].htimer, nodes[i].countdown, interval);
        Serial.println(msgbuff);
#endif
        if (nodes[i].countdown <= 0) {
          if (nodes[i].status == LOST) {
            nodes[i].status = OFFLINE;
            nodes[i].countdown = 0;
            nodes[i].htimer = 0;
          } else {
            nodes[i].status = LOST;
            nodes[i].countdown = (long) nodes[i].htimer * BCT3MULT * 1000;
          }
          displayNode(&nodes[i]);
        }
      }
    }
  }

}

//+
// Analyze a DECNET broadcast packet
// See inline comments
//-
void analyzePacket(uint16_t offset, uint16_t len) {
  char nodename[8];
  int stateChange = 0;
  memset(nodename, 0, 8);
  int dnAddr;
  WORD *etherType;
  struct hello_t *hello;
  struct node_s *node;
  struct frame_s *frame;

#ifdef DEBUG
  dumpPacket(offset, len);
#endif

  // Check if the destination address is one of the DECNET broadcast MACs
  frame = (struct frame_s *) (&ENC28J60::buffer);
  if (memcmp(mCastHelloEN, frame->dst, 6) != 0
      && memcmp(mCastHelloRT, frame->dst, 6) != 0
      && memcmp(mCastL2RT, frame->dst, 6) != 0) return;

  // Check if the ethertype corresponds to DECNET routing,
  // taking into account a posible VLAN tag
  // If it is, set the pointer to the beginning of the 
  // hello payload
  if (frame->u.nonTagged.etherType == ET_DNETROUTING) {
    hello = (struct hello_t *) & (frame->u.nonTagged.payload);
  } else if (frame->u.nonTagged.etherType = ET_VLANTAG) {
    if (frame->u.tagged.etherType == ET_DNETROUTING) {
      hello = (struct hello_t *) & (frame->u.tagged.payload);
    } else {
      return;
    }
  }

  // Check padding bit, advance pointer if necessary
  if ((*((BYTE *)hello) & 0x80) == 0x80) {
    hello = (struct hello_t *) ((char *)hello + 1);
  }

  // Check packet type. If not 5 (ethernet router hello) or
  // 6 (endnode hello), stop analyzing.
  if (hello->routingFlags.type != 6 &&
      hello->routingFlags.type != 5) return;

  dnAddr = hello->dnAddr;
  
  // Lookup the node address in the node list using a dicotomic
  // search. If not found, we are not interested on this node.
  node = dicotomica(dnAddr, 0, numNodes - 1);
  if (node == NULL) return;

  // Evaluate the current state of the node and change
  // it according to the packet.
  // OFFLINE and LOST can change to HELLO
  // HELLO can change to ENDNODE, ROUTER or ROUTER2
  // ENDNODE, ROUTER and ROUTER2 can change between them 
  // (it is not usual, but the EXECUTOR CHAR can be changed).
  // In each case, the countdown is set up to BCT3MULT times the
  // hello timer value. The countdown is maintained in millis,
  // while the hello timer is in seconds.
  switch (node->status) {
    case OFFLINE:
    case LOST:
      node->status = HELLO;
      stateChange = 1;
      if (hello->nodeInfo.nodeType == 3) {
        node->htimer = hello->u.endNode.helloTimer ;
      } else {
        node->htimer = hello->u.router.helloTimer;
      }
      node->countdown = (long) node->htimer * BCT3MULT * 1000;
      break;
    case HELLO:
    case ENDNODE:
    case ROUTER:
      switch (hello->nodeInfo.nodeType) {
        case 3:
          node->htimer = hello->u.endNode.helloTimer;
          if (node->status != ENDNODE) {
            node->status = ENDNODE;
            stateChange = 1;
          }
          break;
        case 2:
          node->htimer = hello->u.router.helloTimer;
          if (node->status != ROUTER) {
            node->status = ROUTER;
            stateChange = 1;
          }
          break;
        case 1:
          node->htimer = hello->u.router.helloTimer;
          if (node->status != ROUTER2) {
            node->status = ROUTER2;
            stateChange = 1;
          }
          break;
      }
      node->countdown = (long) node->htimer * BCT3MULT * 1000;
  }
  // Redisplay the node if it has changed
  if (stateChange == 1) {
#ifdef DEBUG 
      char msgbuff[80];
      sprintf(msgbuff, "Node: %d (%s), ht=%d, cd=%ld, st=%d", node->dnaddr, node->name,
                        node->htimer, node->countdown, node->status);
      Serial.println(msgbuff);
      Serial.flush();
#endif
      displayNode(node);
  }
}

// Extract the decnet address from a MAC address
inline int getDecnetAddress(byte *macPtr) {
  return *(macPtr + 5) * 256 + *(macPtr + 4);
}

// Build a string representation of the DECNET address
void getDecnetName(unsigned int addr, char *buffer) {
  int area = AREA(addr);
  int node = NODE(addr);

  sprintf(buffer, "%d.%d", area, node);
}

//+
// Display a string in the TFT screen
// The coordinates are "text" coordinates, not pixels
// The colors are expressed in a RGB vector of ints,
// which are swapped to please the chinese TFT I've got
//-
void displayString(int col, int fila, char *string, const int *background,
                   const int *color) {

  // Compute the pixel coordinates
  int x = 1 + (6 * FONTWIDTH + 2) * (col - 1);
  int y = 1 + (FONTHEIGHT + 1) * (fila - 1);

  // Draw the background
  screen.noStroke();
  screen.fill(background[2], background[1], background[0]); /* B-G-R */
  screen.rect(x, y, 6 * FONTWIDTH, FONTHEIGHT);

  // Draw the text
  screen.noFill();
  screen.stroke(color[2], color[1], color[0]); /* B-G-R */
  screen.text(string, x, y);
}

//+
// Display a node in the TFT screen
// The node will be colorized according to its
// state. The color values are in the DECnetTFT.h header file
//-
void displayNode(struct node_s *node) {
  const int *back, *color;
  char nodename[7];

  switch (node->status) {
    case OFFLINE:
      back = BKG_OFFLINE;
      color = FG_OFFLINE;
      break;
    case HELLO:
      back = BKG_NEW;
      color = FG_NEW;
      break;
    case ENDNODE:
      back = BKG_STD;
      color = FG_STD;
      break;
    case ROUTER:
      back = BKG_ROUTER;
      color = FG_ROUTER;
      break;
    case ROUTER2:
      back = BKG_ROUTER2;
      color = FG_ROUTER2;
      break;
    case LOST:
      back = BKG_LOST;
      color = FG_LOST;
      break;
    default:
      back = VGA_RED;
      color = VGA_BLACK;
  }
  strncpy(nodename, node->name, 6);
  nodename[6] = '\0';
  displayString(node->ncol, node->nrow, nodename, back, color);
}

//+
// Dicotomic search in the nodelist
// Obviously, the nodelist has to be sorted by
// address.
//-
struct node_s *dicotomica(unsigned int addr, int inici, int fi) {
  int tamany = fi - inici - 1;
  int pivot = inici + tamany / 2;
  struct node_s *nodePtr;

  if ((inici < 0) || (inici > numNodes) || (fi < 0) || (fi >= numNodes)
      || (inici > fi))
    return NULL;

  nodePtr = &nodes[pivot];

  if (nodePtr->dnaddr == addr) {
    return nodePtr;
  } else if (fi == inici) {
    return NULL;
  } else if (nodePtr->dnaddr > addr) {
    return dicotomica(addr, inici, pivot - 1);
  } else {
    return dicotomica(addr, pivot + 1, fi);
  }
}

//+
// Draw the Uptime clock at the bottom of
// the screen
// It show some flicker due to the erasing/redraw.
//-
void displayClock(unsigned long millis) {
  static char line[9];
  int sec, min, hrs;

  unsigned long clock = millis / 1000;
  sec = clock % 60;
  clock /= 60;

  min = clock % 60;
  clock /= 60;
  hrs = clock;

  // Erase previous value
  screen.stroke(0,0,0);
  screen.text(line, 40, 118);

  // Draw new value
  sprintf(line, "%2d:%02d:%02d", hrs, min, sec);
  screen.stroke(0, 225, 225);
  screen.text(line, 40, 118);
}

//+
// Log to serial from flash memory
// nummsg is the index into the message table, defined in
// DecnetTFT.h
//-
void serialmem(int nummsg) {
  char str[25];
  memset(str, 0, 25);
  strncpy_P(str, (char *) pgm_read_word(&(msg_table[nummsg])), 24);
  Serial.println(str);
}

//+
// Display an error message and enter an infinite loop
// nummsg is the index into the message table.
//-
void fatal(int nummsg) {
  char str[25];
  memset(str, 0, 25);
  strncpy_P(str, (char *) pgm_read_word(&(msg_table[nummsg])), 24);

  screen.fill(0,0,0);
  screen.noStroke();
  screen.rect(0, 118, 160, 10);
  
  screen.noFill();
  screen.stroke(0, 0, 204);
  screen.text(str, 0, 118);
  while (true);
}


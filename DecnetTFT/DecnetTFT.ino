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

#include <TFT.h>
#include <EtherCard.h>
#include <enc28j60.h>
#include <SPI.h>
#include <EEPROM.h>



#include "DecnetTFT.h"
#include "LCDConsole.h"

TFT screen = TFT(10, 9, 8);
LCDConsole cons;

// Multicast Addresses
static const byte mCastHelloEN[] = { 0xAB, 0x00, 0x00, 0x03, 0x00, 0x00 };
static const byte mCastHelloRT[] = { 0xAB, 0x00, 0x00, 0x04, 0x00, 0x00 };
static const byte mCastL2RT[]    = { 0x09, 0x00, 0x2b, 0x02, 0x00, 0x00 };
static const byte dnMac[] = { 0xaa, 0x00, 0x04, 0x00, 0xe7, 0x1f };

static struct node_s nodes[16];
int numNodes;

ENC28J60 card;
byte ENC28J60::buffer[128];
unsigned long milliseconds = 0;
unsigned long lastMilliseconds = 0;
unsigned long secondControl = 0;

void setup() {
  int i;

  pinMode(BL_PIN, OUTPUT);
  analogWrite(BL_PIN, 200);
  screen.begin();
  cons.begin(screen);
  cons.println("DECNET listener");

  loadFile(&numNodes, nodes);

  cons.println("LCD");
  Serial.begin(9600);
  screen.background(0, 0, 0);
  screen.stroke(0, 204, 0);
  screen.noFill();
  screen.rect(0, 0, 159, 117);

  Serial.println("ETH");
  card.initSPI();
  if (card.initialize(sizeof Ethernet::buffer, dnMac, ETH_CS) == 0) {
    fatal(ERR01);
  } else {
    card.enableBroadcast();
    card.enableMulticast();
  }
  Serial.println("Ready!");

  Serial.println("Nodes---");
  for (i = 0; i < numNodes; i++) {
    Serial.print(nodes[i].dnaddr);
    Serial.print(": ");
    Serial.println(nodes[i].name);
    displayNode(&nodes[i]);
  }
  Serial.println("End---.");

  screen.stroke(0, 225, 224); // Yellow (B-G-R)
  screen.text("Up:", 0, 118);
  lastMilliseconds = millis();
}

void loop() {

  int i;
  uint16_t len;
  long interval;

  len = card.packetReceive();
  if (len > 0) analyzePacket(0, len);

  milliseconds = millis();

  if (milliseconds - secondControl >= SECOND_MILLIS) {
    secondControl = milliseconds;
    displayClock(secondControl);
  }

  interval = (int) (milliseconds - lastMilliseconds);
  if (interval >= CHECK_MILLIS) {
    lastMilliseconds = milliseconds;

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

  frame = (struct frame_s *) (&ENC28J60::buffer);
  if (memcmp(mCastHelloEN, frame->dst, 6) != 0
      && memcmp(mCastHelloRT, frame->dst, 6) != 0
      && memcmp(mCastL2RT, frame->dst, 6) != 0) return;

  if (frame->u.nonTagged.etherType == ET_DNETROUTING) {
    hello = (struct hello_t *) & (frame->u.nonTagged.payload);
  } else if (frame->u.nonTagged.etherType = ET_VLANTAG) {
    if (frame->u.tagged.etherType == ET_DNETROUTING) {
      hello = (struct hello_t *) & (frame->u.tagged.payload);
    } else {
      return;
    }
  }

  if ((*((BYTE *)hello) & 0x80) == 0x80) {
    hello = (struct hello_t *) ((char *)hello + 1);
  }

  if (hello->routingFlags.type != 6 &&
      hello->routingFlags.type != 5) return;

  dnAddr = hello->dnAddr;

  if (AREA(dnAddr) != MYAREA) return;


  if (hello->routingFlags.type != 6 && hello->routingFlags.type != 5) return;

  node = dicotomica(dnAddr, 0, numNodes - 1);
  if (node == NULL) return;


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


int getDecnetAddress(byte *macPtr) {
  return *(macPtr + 5) * 256 + *(macPtr + 4);
}

void getDecnetName(unsigned int addr, char *buffer) {
  int area = AREA(addr);
  int node = NODE(addr);

  sprintf(buffer, "%d.%d", area, node);
}

void displayString(int col, int fila, char *string, const int *background,
                   const int *color) {

  int x = 1 + (6 * FONTWIDTH + 2) * (col - 1);
  int y = 1 + (FONTHEIGHT + 1) * (fila - 1);

  screen.noStroke();
  screen.fill(background[2], background[1], background[0]); /* B-G-R */
  screen.rect(x, y, 6 * FONTWIDTH, FONTHEIGHT);

  screen.noFill();
  screen.stroke(color[2], color[1], color[0]); /* B-G-R */
  screen.text(string, x, y);
}

void displayNode(struct node_s *node) {
  const int *back, *color;

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

  displayString(node->ncol, node->nrow, node->name, back, color);
}

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

void displayClock(unsigned long millis) {
  static char line[9];
  int sec, min, hrs;

  unsigned long clock = millis / 1000;
  sec = clock % 60;
  clock /= 60;

  min = clock % 60;
  clock /= 60;
  hrs = clock;

  screen.stroke(0,0,0);
  screen.text(line, 40, 118);

  sprintf(line, "%2d:%02d:%02d", hrs, min, sec);
  screen.stroke(0, 225, 225);
  screen.text(line, 40, 118);
}


void fatal(const char *msg) {
  screen.fill(0,0,0);
  screen.noStroke();
  screen.rect(0, 118, 160, 10);
  
  screen.noFill();
  screen.stroke(0, 0, 204);
  screen.text(msg, 0, 118);
  while (true);
}


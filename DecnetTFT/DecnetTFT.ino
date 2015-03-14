#include <EtherCard.h>
#include <enc28j60.h>
#include <SPI.h>
#include <Ucglib.h>

#include "DecnetTFT.h"

//#define DEBUG 1

Ucglib_ST7735_18x128x160_HWSPI ucg(/*cd=*/ 9 , /*cs=10*/ TFT_CS, /*reset=*/ 8);

static const byte dnMac[] = { 0xaa, 0x00, 0x04, 0x00, 0xe7, 0x1f };
static struct node_s nodes[] = {
  { 7 * 1024 + 60, "BITXOV", 0, 0, OFFLINE, 1, 1 },
  { 7 * 1024 + 61, "BITXOO", 0, 0, OFFLINE, 1, 2 },
  { 7 * 1024 + 64, "BITXO1", 0, 0, OFFLINE, 1, 3 },
  { 7 * 1024 + 65, "BITXO2", 0, 0, OFFLINE, 1, 4 },
  { 7 * 1024 + 67, "BITXO4", 0, 0, OFFLINE, 1, 5 },
  { 7 * 1024 + 68, "BITXO5", 0, 0, OFFLINE, 1, 6 },
  { 7 * 1024 + 70, "BITXOT", 0, 0, OFFLINE, 1, 7 },
  { 7 * 1024 + 71, "BITXOR", 0, 0, OFFLINE, 1, 8 },
  { 7 * 1024 + 72, "BITXOM", 0, 0, OFFLINE, 2, 1 },
  { 7 * 1024 + 74, "BITXOW", 0, 0, OFFLINE, 2, 2 },
  { 7 * 1024 + 76, "BITXOX", 0, 0, OFFLINE, 2, 3 },
  { 7 * 1024 + 77, "BITXOY", 0, 0, OFFLINE, 2, 4 },
  { 7 * 1024 + 79, "BITXT0", 0, 0, OFFLINE, 2, 5 },
  { 7 * 1024 + 80, "BITXT1", 0, 0, OFFLINE, 2, 6 },
  { 7 * 1024 + 81, "BITXOZ", 0, 0, OFFLINE, 2, 7 },
  { 7 * 1024 + 82, "BITXOU", 0, 0, OFFLINE, 2, 8 }
};
static int numNodes = 16;

int numPk = 0;
ENC28J60 card;
byte ENC28J60::buffer[512];
unsigned long milliseconds = 0;
unsigned long lastMilliseconds = 0;
unsigned long secondControl = 0;
char msgbuff[80];

void setup() {
  int i;

  pinMode(BL_PIN, OUTPUT);
  analogWrite(BL_PIN, 200);
  // digitalWrite(BL_PIN,HIGH);

  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, LOW);

#ifdef DEBUG
  Serial.begin(9600);
  Serial.println("Decnet HELLO listener");
  Serial.println("Initializing LCD display...");
#endif

  ucg.begin(UCG_FONT_MODE_SOLID);
  ucg.setRotate270();
  ucg.setFontPosTop();
  ucg.clearScreen();
  ucg.setFont(BIG_FONT);
  ucg.setColor(0, 0, 0);
  ucg.drawBox(0, 0, 160, 128);
  ucg.setColor(0, 204, 0);
  ucg.drawFrame(0, 0, 159, 116);

#ifdef DEBUG
  Serial.println("Initializing ethernet device...");
#endif

  card.initSPI();
  if (card.initialize(sizeof Ethernet::buffer, dnMac, ETH_CS) == 0) {
    ucg.setColor(204,0,0);
    ucg.setPrintPos(40, 70);
    ucg.print("Ethernet device not accessible!");
    while (true);
  } else {
    card.enableBroadcast();
    card.enableMulticast();
  }
#ifdef DEBUG
  Serial.println("Ready to go!");
#endif
  for (i = 0; i < numNodes; i++) {
    displayNode(nodes[i]);
  }
  sprintf(msgbuff, "Up:");

  ucg.setFontMode(UCG_FONT_MODE_SOLID);
  ucg.setFont(SMALL_FONT);
  ucg.setColor(0, 225, 225, 0);   /* YELLOW */
  ucg.setColor(1, 0, 0, 0);
  ucg.setPrintPos(60, 118);
  ucg.print(msgbuff);

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
            nodes[i].countdown = nodes[i].htimer * BCT3MULT;
          }
          displayNode(nodes[i]);
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

#ifdef DEBUG
  dumpPacket(offset, len);
   sprintf(msgbuff, "Et: %x", frame->u.nonTagged.etherType); 
   Serial.println(msgbuff);
#endif

  if (frame->u.nonTagged.etherType == ET_DNETROUTING) {
      hello = (struct hello_t *) &(frame->u.nonTagged.payload);
  } else if (frame->u.nonTagged.etherType = ET_VLANTAG) {
    if (frame->u.tagged.etherType == ET_DNETROUTING) {
      hello = (struct hello_t *) &(frame->u.tagged.payload);
    } else {
      return;
    }
  }

  if ((*((BYTE *)hello) & 0x80) == 0x80) {
    hello = (struct hello_t *) ((char *)hello+1);
  }

  if (hello->routingFlags.type != 6 &&
      hello->routingFlags.type != 5) return;

  dnAddr = hello->dnAddr;

#ifdef DEBUG
   sprintf(msgbuff, "dnAddr: %d", dnAddr);
   Serial.println(msgbuff);
#endif

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
          node->htimer = hello->u.endNode.helloTimer * 1000;
      } else {
          node->htimer = hello->u.router.helloTimer * 1000;
      }
      node->countdown = node->htimer * BCT3MULT; 
      break;
    case HELLO:
    case ENDNODE:
    case ROUTER:
      switch (hello->nodeInfo.nodeType) {
        case 3:
          node->htimer = hello->u.endNode.helloTimer * 1000;
          if (node->status != ENDNODE) {
            node->status = ENDNODE;
            stateChange = 1;
          }
          break;
        case 2:
          node->htimer = hello->u.router.helloTimer * 1000;
          if (node->status != ROUTER) {
            node->status = ROUTER;
            stateChange = 1;
          }
          break;
        case 1:
          node->htimer = hello->u.router.helloTimer * 1000;
          if (node->status != ROUTER2) {
            node->status = ROUTER2;
            stateChange = 1;
          }
          break;
      }
      node->countdown = node->htimer * BCT3MULT;
  }
#ifdef DEBUG
        sprintf(msgbuff, "Node: %d (%s), ht=%ld, cd=%ld",node->dnaddr, node->name,
                node->htimer, nodes->countdown);
        Serial.println(msgbuff);
#endif
  if (stateChange == 1) displayNode(*node);
}

void printHexByte(int b) {
  int high = b / 16;
  int low = b % 16;
  Serial.print(high, HEX);
  Serial.print(low, HEX);
}

#ifdef DEBUG
void dumpPacket(int offset, int len) {
  int i, j, k;
  int c;
  char ascii[33];
  memset(ascii, 0, 33);
  Serial.print("Paquet rebut (");
  Serial.print(numPk++);
  Serial.print("), longitud=");
  Serial.print(len);
  Serial.println(" bytes.");
  j = 0;
  for (i = 0; i < len; i++) {
    if (j < 32) {
      c = ENC28J60::buffer[offset + i];
      Serial.print(" ");
      printHexByte(c);
      if (isprint(c)) {
        ascii[j] = c;
      } else {
        ascii[j] = '.';
      }
      j += 1;
    } else {
      j = 0;
      Serial.print(" ");
      Serial.println(ascii);
      memset(ascii, 0, 33);
    }
  }
  for (; j < 32; j++) {
    Serial.print("   ");
  }
  Serial.print(" ");
  Serial.println(ascii);
}
#endif

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

  ucg.setFontMode(UCG_FONT_MODE_SOLID);
  ucg.setFont(BIG_FONT);
  ucg.setColor(0, color[0], color[1], color[2]);
  ucg.setColor(1, background[0], background[1], background[2]);
  ucg.setPrintPos(x, y);
  ucg.print(string);
}

void displayNode(struct node_s &node) {
  const int *back, *color;

  switch (node.status) {
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

  displayString(node.dpyX, node.dpyY, node.name, back, color);
}

struct node_s *dicotomica(unsigned int addr, int inici, int fi) {
  int tamany = fi - inici - 1;
  int pivot = inici + tamany / 2;
  struct node_s *nodePtr;

  if ((inici < 0) || (inici > numNodes) || (fi < 0) || (fi >= numNodes)
      || (inici > fi))
    return NULL;

#ifdef DEBUG
  Serial.print(inici); Serial.print(" ");
  Serial.print(fi); Serial.print(" ");
  Serial.print(addr); Serial.print(" | ");
#endif

  nodePtr = &nodes[pivot];

#ifdef DEBUG
  Serial.print(pivot); Serial.print("->");
  Serial.print(nodePtr->dnaddr); Serial.print(":"); Serial.println(nodePtr->name);
#endif

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
  char line[80];
  int sec, min, hrs;

  unsigned long clock = millis / 1000;
  sec = clock % 60;
  clock /= 60;

  min = clock % 60;
  clock /= 60;

  hrs = clock;

  sprintf(line, "%2d:%02d:%02d", hrs, min, sec);

  ucg.setFontMode(UCG_FONT_MODE_SOLID);
  ucg.setFont(SMALL_FONT);
  ucg.setColor(0, 225, 225, 0);   /* YELLOW */
  ucg.setColor(1, 0, 0, 0);
  ucg.setPrintPos(90, 118);
  ucg.print(line);
}

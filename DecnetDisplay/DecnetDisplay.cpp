// Do not remove the include below
#include "DecnetDisplay.h"
#include <EtherCard.h>
#include <enc28j60.h>
#include <UTFT.h>

/**

 Prototype board header pins
 ===========================

 *
 * Display module
 *
 01-07 -> DB15-DB8 -> 29-22
 08 (NC)
 09 ->
 10 ->
 11 ->
 12 ->
 13 ->

 *
 * Ethernet module
 *
 14 -> CS  -> 53
 15 -> SI  -> 51
 16 -> SO  -> 50
 17 -> SCK -> 52

 *
 * Touch interface
 *

 *
 * Power
 *
 22 -> LED backlight -> 5v or 3v
 24 -> 3.3V	-> 3.3V
 26 -> GND -> GND

 **/

UTFT lcd(ILI9325C, 41, 40, 39, 38); // Remember to change the model parameter to suit your display module!

extern uint8_t BigFont[];
extern uint8_t SmallFont[];
static const byte dnMac[] = { 0xaa, 0x00, 0x04, 0x00, 0xe7, 0x1f };
static const byte mCastHelloEN[] = { 0xAB, 0x00, 0x00, 0x03, 0x00, 0x00 };
static const byte mCastHelloRT[] = { 0xAB, 0x00, 0x00, 0x04, 0x00, 0x00 };
static struct node_s nodes[] = {
		{ 7 * 1024 + 6, "MBSERV",  0, OFFLINE, 1, 1 },
		{ 7 * 1024 + 60, "BITXOV", 0, OFFLINE, 1, 2 },
		{ 7 * 1024 + 61, "BITXOO", 0, OFFLINE, 1, 3 },
		{ 7 * 1024 + 64, "BITXO1", 0, OFFLINE, 1, 4 },
		{ 7 * 1024 + 65, "BITXO2", 0, OFFLINE, 1, 5 },
		{ 7 * 1024 + 67, "BITXO4", 0, OFFLINE, 1, 6 },
		{ 7 * 1024 + 68, "BITXO5", 0, OFFLINE, 2, 1 },
		{ 7 * 1024 + 70, "BITXOT", 0, OFFLINE, 2, 2 },
		{ 7 * 1024 + 71, "BITXOR", 0, OFFLINE, 2, 3 },
		{ 7 * 1024 + 72, "BITXOM", 0, OFFLINE, 2, 4 },
		{ 7 * 1024 + 74, "BITXOW", 0, OFFLINE, 2, 5 },
		{ 7 * 1024 + 76, "BITXOX", 0, OFFLINE, 2, 6 },
		{ 7 * 1024 + 77, "BITXOY", 0, OFFLINE, 3, 1 },
		{ 7 * 1024 + 79, "BITXT0", 0, OFFLINE, 3, 2 },
		{ 7 * 1024 + 80, "BITXT1", 0, OFFLINE, 3, 3 },
		{ 7 * 1024 + 81, "BITXOZ", 0, OFFLINE, 3, 4 },
		{ 7 * 1024 + 82, "BITXOU", 0, OFFLINE, 3, 5 } };
static int numNodes = 17;

int numPk = 0;
ENC28J60 card;
byte ENC28J60::buffer[512];
unsigned long milliseconds = 0;
unsigned long lastMilliseconds = 0;
unsigned long secondControl = 0;
int ledStatus = 0;
long cycle = 0L;

void setup() {
	int i;

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);

	Serial.begin(57600);
	Serial.println("Decnet HELLO listener");

	Serial.println("Initializing LCD display...");
	lcd.InitLCD();
	lcd.setFont(BigFont);
	lcd.clrScr();
	lcd.setColor(VGA_GREEN);
	lcd.setBackColor(VGA_BLACK);
	lcd.drawRect(0, 0, 319, 199);

	Serial.println("Initializing ethernet device...");
	card.initSPI();
	if (card.initialize(sizeof Ethernet::buffer, dnMac, 53) == 0) {
		Serial.println("Ethernet device not accessible!");
		while (true)
			;
	} else {
		card.enableBroadcast();
		card.enableMulticast();
	}
	Serial.println("Ready to go!");

	for (i = 0; i < numNodes; i++) {
		displayNode(lcd, nodes[i]);
	}
	lastMilliseconds = millis();

}

void loop() {
	int i;
	uint16_t len;
	long interval;
	char msgbuff[80];

	len = card.packetReceive();
	if (len > 0) analyzePacket(0, len);

	milliseconds = millis();

	ledStatus = (secondControl - milliseconds) * 512 / 1000;
	if (ledStatus > 256)
		ledStatus = 512 - ledStatus;
	analogWrite(LED_BUILTIN, ledStatus);

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
				sprintf(msgbuff,"Node: %d (%s), cd=%ld, intv=%ld", i, nodes[i].name,
				 		nodes[i].countdown, interval);
				Serial.println(msgbuff);
#endif
				if (nodes[i].countdown <= 0) {
					if (nodes[i].status == LOST) {
						nodes[i].status = OFFLINE;
						nodes[i].countdown = 0;
					} else {
						nodes[i].status = LOST;
						nodes[i].countdown = CYCLE_MILLIS;
					}
					displayNode(lcd, nodes[i]);
				}
			}
		}
	}
}

void analyzePacket(uint16_t offset, uint16_t len) {
	char nodename[8];
	memset(nodename, 0, 8);
	int dnAddr;
	WORD *etherType;
	struct hello_t *hello;
	struct node_s *node;
	struct frame_s *frame;

	frame = (struct frame_s *) (&ENC28J60::buffer);
	if (memcmp(mCastHelloEN, frame->dst, 6) != 0
			&& memcmp(mCastHelloRT, frame->dst, 6) != 0) return;

	etherType = (WORD *) ((BYTE *) &(ENC28J60::buffer) + OFS_ETHERTYPE);
	if (*etherType == ET_VLANTAG) etherType++;
	if (*etherType != ET_DNETROUTING) return;


	hello = (struct hello_t *) ((BYTE *) etherType + OFS_FRAMESIZE + OFS_FRAME);
	if ((*((BYTE *) hello) & 0x80) == 0x80) {
		hello = (struct hello_t *) ((BYTE *) hello + 1);
	}
	dnAddr = hello->dnAddr;

	if (AREA(dnAddr) != MYAREA) return;
	if (hello->routingFlags.type != 6 && hello->routingFlags.type != 5) return;

	node = dicotomica(dnAddr, 0, numNodes - 1);
	if (node == NULL) return;

	node->countdown = CYCLE_MILLIS;

	switch (node->status) {
		case OFFLINE:
		case LOST:
			node->status = HELLO;
			break;
		case HELLO:
		case ENDNODE:
		case ROUTER:
			switch (hello->nodeInfo.nodeType) {
				case 3:
					node->status = ENDNODE;
					break;
				case 2:
					node->status = ROUTER;
					break;
				case 1:
					node->status = ROUTER2;
					break;
			}
	}
	displayNode(lcd, *node);
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

void displayString(UTFT &lcd, int col, int fila, char *string, int background,
		int color) {

	int x = 1 + (6 * FONTWIDTH + 2) * (col - 1);
	int y = 1 + (FONTHEIGHT + 1) * (fila - 1);

	lcd.setBackColor(background);
	lcd.setColor(color);
	lcd.print(string, x, y);
}

void displayNode(UTFT &lcd, struct node_s &node) {
	int back, color;

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

	displayString(lcd, node.dpyX, node.dpyY, node.name, back, color);
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

	sprintf(line, "Up: %4d:%02d:%02d", hrs, min, sec);

	lcd.setFont(SmallFont);
	lcd.setBackColor(VGA_GRAY);
	lcd.setColor(VGA_YELLOW);
	lcd.print(line, 208, 202);
	lcd.setFont(BigFont);
}

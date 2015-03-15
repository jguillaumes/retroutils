#include <EtherCard.h>
#include <enc28j60.h>


extern ENC28J60 card;

static int numPk = 0;

void printHexByte(int b) {
  int high = b / 16;
  int low = b % 16;
  Serial.print(high, HEX);
  Serial.print(low, HEX);
}

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



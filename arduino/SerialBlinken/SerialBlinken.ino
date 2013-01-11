#include <SoftwareSerial.h>
#include <Wire.h>

/*
 * Payload (bits) flags
 */
#define BLF_NOPARITY 0x0001
#define BLF_ERROR    0x0002
#define BLF_TEST     0x0004

#pragma pack(2)
struct s_payload {
  unsigned short          bFlags;
  unsigned short          numDataBytes;
  unsigned short          numAddrBytes;
  unsigned short          numOtherBytes;
  unsigned char          data[32];
};
#pragma pack()

int DATA=10;
int CLOCK=11;
int LATCH=12;

int nval=0;
unsigned char val[32];

unsigned short ntohs(unsigned short input) {
  return word(lowByte(input), highByte(input));
}


void liteDataBytes(int stat, int numBytes, unsigned char *v) {
  unsigned char *ptr = v;
  
  digitalWrite(LATCH,LOW);
  shiftOut(DATA,CLOCK,MSBFIRST,stat);
  for (int i=0; i<numBytes; i++) {
    shiftOut(DATA,CLOCK, MSBFIRST, *ptr++);
  }
  digitalWrite(LATCH,HIGH);
}

int parity(unsigned char c) {
  int p=0;
  int w=c;
  
  while(w > 0) {
    p += w % 2;
    w >>= 1;
  }
  return p % 2;
}

unsigned int blockParity(int n, unsigned char *v) {
  unsigned char *ptr=v;
  unsigned char p=0;
  unsigned int par=0;
  
  for(int i=0; i<n; i++) {
    p += parity(*ptr++);
  }
  return p % 2;
}

void setup() {
  pinMode(DATA,OUTPUT);
  pinMode(CLOCK,OUTPUT);
  pinMode(LATCH,OUTPUT);
  Serial.begin(57600);
  for(int i=0; i<32; i++) {
    val[i] = 0;
  }
}

void loop() {
  long i=0;
  unsigned int f=0,p=0,w=0;
  unsigned char inp[sizeof(struct s_payload)];
  unsigned char *dataArray  = NULL;
  unsigned char *addrArray  = NULL;
  unsigned char *otherArray = NULL;

  while(Serial.available() > 0) {
    i = Serial.readBytes((char *)inp, sizeof(struct s_payload));
    struct s_payload *payload;

    payload = (struct s_payload *) &inp;

    payload->bFlags = ntohs(payload->bFlags);    
    payload->numDataBytes = ntohs(payload->numDataBytes);
    payload->numAddrBytes = ntohs(payload->numAddrBytes);
    payload->numOtherBytes = ntohs(payload->numOtherBytes);

    dataArray  = &(payload->data[0]);
    addrArray  = dataArray + payload->numDataBytes;
    otherArray = addrArray + payload->numOtherBytes;

    if (payload->numAddrBytes == 0)  addrArray = NULL;
    if (payload->numOtherBytes == 0) otherArray = NULL;

    if ((payload->bFlags & BLF_TEST) != 0) {
      for (int i=0; i<payload->numDataBytes; i++) {
        val[i] = 255;
      }
      liteDataBytes(255, payload->numDataBytes, val);
    } 
    else if ((payload->bFlags & BLF_ERROR) != 0) {
      liteDataBytes(2, payload->numDataBytes, val);
    } 
    else {
      for (int i=0; i<payload->numDataBytes; i++) {
        val[i] = payload->data[i];
        if ((payload->bFlags & BLF_NOPARITY) == 0) {
          p = blockParity(payload->numDataBytes, val);
        } 
        else {
          p = 0;
        }
      }
      liteDataBytes(p,payload->numDataBytes, val);
    }
  }
}


/**
Connexió plaqueta blinkenlights

Leonardo/Micro:

Groc: MOSI
Vermell: D	12
Taronja: CLK

Marró: GROUND
Verd: +5/+3.3V

================

UNO:

================

MEGA:

**/


#include <SPI.h>
#include <util/parity.h>
/*
 * Payload (bits) flags
 */
#define BLF_NOPARITY 0x0001
#define BLF_ERROR    0x0002
#define BLF_TEST     0x0004
#define TTY_SPEED    230400
#define LED_PIN      13
#define TEST_BUTTON  2

#define XON   0x11
#define XOFF  0x13

#pragma pack(2)
struct s_payload {
  unsigned short          bFlags;
  unsigned short          numDataBytes;
  unsigned short          numAddrBytes;
  unsigned short          numOtherBytes;
  unsigned char          data[16];
};
#pragma pack()

const int LATCH=12;

int nval=0;
long ledstate=0;
int leddirection=0;
unsigned char val[16];
unsigned char err[3];
struct s_payload *payload;
int testMode = 0;

/*
** Setup the arduino board
 */
void setup() {
  pinMode(LED_PIN,OUTPUT);
  pinMode(LATCH,OUTPUT);
  pinMode(TEST_BUTTON,INPUT_PULLUP);
  SPI.begin();
  Serial.begin(TTY_SPEED);
  Serial.setTimeout(5000);
  attachInterrupt(1, buttonAction, CHANGE);
  for(int i=0; i<16; i++) {
    val[i] = 0;
  }
  err[0] = 255;
  err[1] = 255;
  err[2] = 255;
}


/*
* Interrupt routine to handle the "press to test" button
 */
void buttonAction() {
  if (digitalRead(TEST_BUTTON) == LOW) {
    testMode = 1;
  } else {
    testMode = 0;
  }  
}

unsigned short ntohs(unsigned short input) {
  return word(lowByte(input), highByte(input));
}

/*
** Turn on the LEDs according to a byte array
 */
void liteDataBytes(int stat, int numBytes, unsigned char *v) {
  unsigned char *ptr = v;

  digitalWrite(LATCH,LOW);
  SPI.transfer(stat);
  for (int i=0; i<numBytes; i++) {
    SPI.transfer(*ptr++);
  }
  digitalWrite(LATCH,HIGH);
}

/*
** Compute the parity for a character
 */
int parity(unsigned char c) {
  int p=0;
  int w=c;

  while(w > 0) {
    p += w % 2;
    w >>= 1;
  }
  return p % 2;
}


/*
** Main loop
 */
void loop() {
  unsigned char inp[sizeof(struct s_payload)];
  blinkLed();    // "I'm alive" heartbeat

  while(Serial.available() > 0) {
    Serial.readBytes((char *)inp, sizeof(struct s_payload));
    if (testMode == 0) {
      payload = (struct s_payload *) &inp;
      processPayload();
    } 
    else {
      liteDataBytes(255,2,err);
    } 
  }
}


void processPayload() {
  unsigned int f=0,p=0,w=0;
  unsigned char *dataArray  = NULL;
  unsigned char *addrArray  = NULL;
  unsigned char *otherArray = NULL;

  payload->bFlags        = ntohs(payload->bFlags);    
  payload->numDataBytes  = ntohs(payload->numDataBytes);
  payload->numAddrBytes  = ntohs(payload->numAddrBytes);
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
    liteDataBytes(4, payload->numDataBytes, val);
  } 
  else {
    for (int i=0; i<payload->numDataBytes; i++) {
      val[i] = payload->data[i];
      if ((payload->bFlags & BLF_NOPARITY) == 0) {
        p = 2*p + parity_even_bit(val[i]);
      } 
      else {
        p = 0;
      }
    }
    liteDataBytes(p,payload->numDataBytes, val);
  }
}

/*
** Heartbeat in on-board LED
 */
void blinkLed() {
    analogWrite(LED_PIN, ledstate / 20);
  if (leddirection == 0) {
    ledstate++;
    if (ledstate > 5120) {
      leddirection = 1;
    }
  } else {
    ledstate--;
    if (ledstate <= 0) {
      leddirection = 0;
    } 
  }
}




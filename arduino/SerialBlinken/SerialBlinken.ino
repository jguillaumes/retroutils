#include <SoftwareSerial.h>
#include <Wire.h>

int DATA=10;
int CLOCK=11;
int LATCH=12;

unsigned char val[2];

void liteBytes(unsigned int f, unsigned char v[2]) {
  digitalWrite(LATCH,LOW);
  shiftOut(DATA,CLOCK, MSBFIRST, f);
  shiftOut(DATA,CLOCK, MSBFIRST, v[0]);
  shiftOut(DATA,CLOCK, MSBFIRST, v[1]);
  digitalWrite(LATCH,HIGH);
}

unsigned int parity(unsigned int n) {
  unsigned int p=0,r=0;
  
  r=n;
  while(r!=0) {
    p += r%2;
    r >>= 1;
  }
  p= p%2;
  Serial.print("Parity: "); Serial.println(p);
  return p;
}

void setup() {
  pinMode(DATA,OUTPUT);
  pinMode(CLOCK,OUTPUT);
  pinMode(LATCH,OUTPUT);
  Serial.begin(57600);
  val[0] = 0;
  val[1] = 0;
  liteBytes(0, val);
}

void loop() {
  long i=0;
  unsigned int f=0,p=0,w=0;
  unsigned char inp[2];

  while(Serial.available() > 0) {
    f = Serial.read();
    i = Serial.readBytes((char *)inp, 2);
    
    if (bitRead(f,2)==1) {
      val[0] = 0xFF;
      val[1] = 0xFF;
      liteBytes(255, val);
    } else if (bitRead(f,0)==1) {
      liteBytes(0x02, val);
    } else {
      val[0] = inp[0];
      val[1] = inp[1];
      if (bitRead(f,1)!=1) {
        w = val[0]>>8 | val[1];
        p = parity(w);
      } else {
        p = 0;
      }
      liteBytes(p,val);
    }
  }
}



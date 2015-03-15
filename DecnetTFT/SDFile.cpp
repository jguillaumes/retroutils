#include <TFT.h>

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

#include <EEPROM.h>
#include <SdFat.h>
#include <SPI.h>
#include "DecnetTFT.h"
#include "LCDConsole.h"

#define MAX_EEPROM 512
#define EEPROM_START     0
#define EEPROM_EYE      (EEPROM_START + 0)
#define EEPROM_NUMNODES (EEPROM_EYE + 4)
#define EEPROM_BASE     (EEPROM_NUMNODES + sizeof(int))

SdFat SD;

const static BYTE EYE[4] = { 0xEE, 0x11, 0xEE, 0x55 };
extern LCDConsole cons;

bool loadFileSD(int *numNodes, struct node_s *nodes);
bool loadFileEEPROM(int *numNodes, struct node_s *nodes);
void saveEEPROM(int numNodes, struct node_s *nodes);

int numRow(int pos) {
  return (pos % LINES) + 1;
}

int numCol(int pos) {
  return (pos / LINES) + 1;
}

bool loadFile(int *numNodes, struct node_s *nodes) {
  if (!loadFileSD(numNodes, nodes)) {
    if (!loadFileEEPROM(numNodes, nodes)) {
      fatal(ERR04);
    }
  }
}

bool loadFileSD(int *numNodes, struct node_s *nodes) {
  ifstream nodeFile;
  char buffer[32];
  String line;
  int area,nodenum,nc;
  char str[16];
  struct node_s *node, *nodetable;
  int i,j;
  
  cons.println("Init SCD");
  if (!SD.begin(SD_CS,SPI_HALF_SPEED)) {
    //if (!SD.begin(SD_CS)) {
    cons.println(ERR00);
    return false;
  }
  
  cons.println("Open nodefile...");
  nodeFile.open("nodes.dat");
  if (!nodeFile.is_open()) {
    cons.println(ERR02);
    return false;
  }
  cons.println("Open OK");
  
  // nodeFile.readBytesUntil('\r', buffer, 40);
  nodeFile >> *numNodes; nodeFile.ignore();
  sprintf(buffer, "Numnodes: %d", *numNodes);
  cons.println(buffer);
  
  memset((void *)nodes, 0, *numNodes * sizeof(struct node_s));
  
  cons.println("Read file...");
  for (i=0; i < *numNodes; i++) {
    
    nodeFile.get(str, 16, '.'); nodeFile.ignore();
    area = atoi(str);
    
    nodeFile.get(str, 16, ','); nodeFile.ignore();
    nodenum =  atoi(str);
    
    nodes[i].dnaddr = 1024 * area + nodenum;
        
    nodeFile.get(str, 16, '\n'); nodeFile.ignore();
    strncpy(nodes[i].name, str, 6);

    nodes[i].htimer = 0;
    nodes[i].countdown = 0;
    nodes[i].status = OFFLINE;
    nodes[i].ncol = numCol(i);
    nodes[i].nrow = numRow(i);
  }
  nodeFile.close();
  // SPI.setClockDivider(SPI_CLOCK_DIV4);
  saveEEPROM(*numNodes, nodes);
  cons.println("File OK");
  return true;
}

bool loadFileEEPROM(int *numNodes, struct node_s *nodes) {
  char buffer[16];
  BYTE l,h;
  int i,j, eadr;
  byte check;

  for (i=0; i<4; i++) {
    l = EEPROM.read(EEPROM_EYE+i);
    if (l != EYE[i]) {
      cons.println(ERR05);
      return false;
    }
  }     
    
  l = EEPROM.read(EEPROM_NUMNODES);
  h = EEPROM.read(EEPROM_NUMNODES+1);
  
  *numNodes = l + h * 256;
  sprintf(buffer, "numNodes: %d", *numNodes);
  cons.println(buffer);

  memset((void *)nodes, 0, *numNodes * sizeof(struct node_s));


  eadr = EEPROM_BASE;
  for (i=0; i<*numNodes; i++) {
    l = EEPROM.read(eadr++);
    h = EEPROM.read(eadr++);
    nodes[i].dnaddr = l + 256 * h;
    for (j=0; j<6; j++) {
      nodes[i].name[j] = EEPROM.read(eadr++);
    } 
    nodes[i].htimer = 0;
    nodes[i].countdown = 0;
    nodes[i].status = OFFLINE;
    nodes[i].ncol = numCol(i);
    nodes[i].nrow = numRow(i);
  }
}

void saveEEPROM(int numNodes, struct node_s *nodes) {
  cons.println("EEPROM wrt...");
  char buffer[16];
  BYTE l,h;
  int i,j, eadr;
  byte check;

  for (i=0; i<4; i++) {
    if (EEPROM.read(EEPROM_EYE+i) != EYE[i]) EEPROM.write(EEPROM_EYE+i, EYE[i]);
  }     
    
  l = numNodes % 256;
  h = numNodes / 256;
  if (EEPROM.read(EEPROM_NUMNODES) != l) EEPROM.write(EEPROM_NUMNODES, l);
  if (EEPROM.read(EEPROM_NUMNODES+1) != h) EEPROM.write(EEPROM_NUMNODES+1, h);
  
  eadr = EEPROM_BASE;
  for (i=0; i<numNodes; i++) {
    l = nodes[i].dnaddr % 256;
    if (EEPROM.read(eadr) != l) EEPROM.write(eadr++, l);
    h = nodes[i].dnaddr / 256;
    if (EEPROM.read(eadr) != h) EEPROM.write(eadr++, h);
    for (j=0; j<6; j++) {
      if (EEPROM.read(eadr) != (BYTE) nodes[i].name[j])
        EEPROM.write(eadr++, (BYTE) nodes[i].name[j]);
    } 
  }
  cons.println("EEPROM OK");
}


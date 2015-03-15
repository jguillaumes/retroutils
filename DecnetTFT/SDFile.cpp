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

#include <SdFat.h>
#include <SPI.h>
#include "DecnetTFT.h"
#include "LCDConsole.h"

SdFat SD;

extern LCDConsole cons;

int numRow(int pos) {
  return (pos % LINES) + 1;
}

int numCol(int pos) {
  return (pos / LINES) + 1;
}


bool loadFile(int *numNodes, struct node_s *nodes) {
  ifstream nodeFile;
  char buffer[40];
  String line;
  int area,nodenum,nc;
  char str[16];
  struct node_s *node, *nodetable;
  int i,j;
  
  
  
  cons.println("Init SCD");
  if (!SD.begin(SD_CS,SPI_HALF_SPEED)) {
    //if (!SD.begin(SD_CS)) {
    cons.println(ERR00);
    fatal(ERR00);
  }
  
  cons.println("Opening nodefile...");
  nodeFile.open("nodes.dat");
  if (!nodeFile.is_open()) {
    cons.println(ERR02);
    fatal(ERR02);
  }
  cons.println("SD Open OK");
  
  // nodeFile.readBytesUntil('\r', buffer, 40);
  nodeFile >> *numNodes; nodeFile.ignore();
  sprintf(buffer, "Numnodes: %d", *numNodes);
  cons.println(buffer);
  
  memset((void *)nodes, 0, *numNodes * sizeof(struct node_s));
  
  cons.println("Reading file...");
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
  cons.println("Nodefile OK");

}

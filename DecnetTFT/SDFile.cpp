/////////////////////////////////////////////////////////////////////////////
// DECNET Hello listener V01.00                                            //
/////////////////////////////////////////////////////////////////////////////
// Nodelist loading
//
// To load the nodelist:
// - First, we try to load it from a SD card containing a NODES.DAT file.
//   See the comments in the main file (DecnetTFT.ino) for details about
//   its format.
// - If that fails, we try to read the nodelist from EEPROM.
//
// When we load the list from NODES.DAT we update the EEPROM with the
// new information.
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

// Offsets into EEPROM space
#define EEPROM_START     0
#define EEPROM_EYE      (EEPROM_START + 0)
#define EEPROM_NUMNODES (EEPROM_EYE + 4)
#define EEPROM_BASE     (EEPROM_NUMNODES + sizeof(int))

// Eyecatcher for EEPROM table
const static BYTE EYE[4] = { 0xEE, 0x11, 0xEE, 0x55 };

SdFat SD;
extern LCDConsole cons;

bool loadFileSD(int *numNodes, struct node_s *nodes);
bool loadFileEEPROM(int *numNodes, struct node_s *nodes);
void saveEEPROM(int numNodes, struct node_s *nodes);

// Functions to assign text coordinates to a node
// according to its position in the nodelist
int numRow(int pos) {
  return (pos % LINES) + 1;
}

int numCol(int pos) {
  return (pos / LINES) + 1;
}

//+
// Entry point, called by the main sketch file
//-
bool loadFile(int *numNodes, struct node_s *nodes) {
  if (!loadFileSD(numNodes, nodes)) {          // Try SD first
    if (!loadFileEEPROM(numNodes, nodes)) {    // Try EEPROM secons
      fatal(ERR04);                            // Bad luck, no cookie
    }
  }
}

//+
// Load the nodelist from SD
//-
bool loadFileSD(int *numNodes, struct node_s *nodes) {
  ifstream nodeFile;
  char buffer[32];
  String line;
  int area,nodenum,nc;
  char str[16];
  struct node_s *node, *nodetable;
  int i,j;
  
  // Initialize the SD in half speed mode (I've noticed FULL_SPEED
  // does not always work).
  // If the initialization fails, we return false so our caller will
  // probably try EEPROM.
  cons.printmem(INITSD);
  if (!SD.begin(SD_CS,SPI_HALF_SPEED)) {
    cons.printmem(ERR00);
    return false;
  }
  
  // Open the NODES.DAT file. If the open fails, return false
  // to trigger EEPROM read.
  cons.printmem(OPENNF);
  nodeFile.open("nodes.dat");
  if (!nodeFile.is_open()) {
    cons.printmem(ERR02);
    return false;
  }
  cons.printmem(OPENOK);
  
  // Get the number of nodes from the first line
  nodeFile >> *numNodes; nodeFile.ignore();
  sprintf(buffer, "Numnodes: %d", *numNodes);
  cons.println(buffer);
  
  // Clean up the part of the table we will use
  memset((void *)nodes, 0, *numNodes * sizeof(struct node_s));
  
  cons.printmem(READFIL);
  
  // Read the nodes according to the number of nodes
  // WARNING; There is no check, formal or otherwise.
  // If NODES.DAT is not consistent bad things WILL happen
  for (i=0; i < *numNodes; i++) {

    // Get the area number    
    nodeFile.get(str, 16, '.'); nodeFile.ignore();
    area = atoi(str);
    // Get the node number
    nodeFile.get(str, 16, ','); nodeFile.ignore();
    nodenum =  atoi(str);
    // Build the decnet address
    nodes[i].dnaddr = 1024 * area + nodenum;
    // Get the node name and store it in the table
    nodeFile.get(str, 16, '\n'); nodeFile.ignore();
    strncpy(nodes[i].name, str, 6);

    // Complete the entry with default values, and
    // compute the text coordinates 
    nodes[i].htimer = 0;
    nodes[i].countdown = 0;
    nodes[i].status = OFFLINE;
    nodes[i].ncol = numCol(i);
    nodes[i].nrow = numRow(i);
  }
  nodeFile.close();
  
  // Load done: save the stuff to EEPROM
  saveEEPROM(*numNodes, nodes);
  cons.printmem(READOK);
  return true;
}

//+
// Load nodelist from EEPROM
//-
bool loadFileEEPROM(int *numNodes, struct node_s *nodes) {
  char buffer[16];
  BYTE l,h;
  int i,j, eadr;
  byte check;

  cons.printmem(RDEPROM);
  
  // Check eyecatcher. If not correct, we have failed!
  for (i=0; i<4; i++) {
    l = EEPROM.read(EEPROM_EYE+i);
    if (l != EYE[i]) {
      cons.printmem(ERR05);
      return false;
    }
  }     
  
  // Get the number of nodes  
  l = EEPROM.read(EEPROM_NUMNODES);
  h = EEPROM.read(EEPROM_NUMNODES+1);
  
  *numNodes = l + h * 256;
  sprintf(buffer, "numNodes: %d", *numNodes);
  cons.println(buffer);
  
  // Clean up the part of the table we will use
  memset((void *)nodes, 0, *numNodes * sizeof(struct node_s));


  // Load the nodes from EEPROM (addresses and names)
  eadr = EEPROM_BASE;
  for (i=0; i<*numNodes; i++) {
    l = EEPROM.read(eadr++);
    h = EEPROM.read(eadr++);
    nodes[i].dnaddr = l + 256 * h;
    for (j=0; j<6; j++) {
      nodes[i].name[j] = EEPROM.read(eadr++);
    } 
    // Complete the entry with default values and compute
    // the text coordinates.
    nodes[i].htimer = 0;
    nodes[i].countdown = 0;
    nodes[i].status = OFFLINE;
    nodes[i].ncol = numCol(i);
    nodes[i].nrow = numRow(i);
  }
  cons.printmem(OKEPROM);  // Yay! We did it!
}

//+
// Save the node table to EEPROM
// Before each write we will check if the value has actually
// changed to save erase cycles.
//-
void saveEEPROM(int numNodes, struct node_s *nodes) {
  cons.printmem(WREPROM);
  char buffer[16];
  BYTE l,h;
  int i,j, eadr;
  byte check;
  
  // Write the eyecatcher
  for (i=0; i<4; i++) {
    if (EEPROM.read(EEPROM_EYE+i) != EYE[i]) EEPROM.write(EEPROM_EYE+i, EYE[i]);
  }     
    
  // Decompose and write the number of nodes
  l = numNodes % 256;
  h = numNodes / 256;
  if (EEPROM.read(EEPROM_NUMNODES) != l) EEPROM.write(EEPROM_NUMNODES, l);
  if (EEPROM.read(EEPROM_NUMNODES+1) != h) EEPROM.write(EEPROM_NUMNODES+1, h);
  
  // Write the node table
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
  cons.printmem(OKEPROM);;
}


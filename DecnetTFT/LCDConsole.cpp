/////////////////////////////////////////////////////////////////////////////
// DECNET Hello listener V01.00                                            //
/////////////////////////////////////////////////////////////////////////////
// Dumb console over the TFT
//

#include "LCDConsole.h"
#include "DecnetTFT.h"

LCDConsole::LCDConsole() {}

// Initialize the "console"
bool LCDConsole::begin(TFT &tft) {
    row = 0;
    theTFT = &tft;
    theTFT->background(0,0,0);
    return true;
}

// Output a message, passed as C string
bool LCDConsole::println(const char *msg) {
  if (row > 10) row = 0;
  theTFT->noStroke();
  theTFT->fill(0,0,0);
  theTFT->rect(0,12*row,160,12*(row+1));
  theTFT->noFill();
  theTFT->stroke(255,255,255);
  theTFT->text(msg,0,12*row);
  row++;
  return true;
}

// Output a message, passed as C++ String
bool LCDConsole::println(String &msg) {
  return println(msg.c_str());
}

// Output a message, passed as index into the
// message table residing in FLASH memory
bool LCDConsole::printmem(int nummsg) {
  char str[25];
  memset(str, 0, 25);
  strncpy_P(str, (char *) pgm_read_word(&(msg_table[nummsg])), 24);
  println(str);
}


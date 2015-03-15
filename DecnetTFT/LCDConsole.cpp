#include "LCDConsole.h"

LCDConsole::LCDConsole() {}

bool LCDConsole::begin(TFT &tft) {
    row = 0;
    theTFT = &tft;
    theTFT->background(0,0,0);
    return true;
}

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

bool LCDConsole::println(String &msg) {
  return println(msg.c_str());
}



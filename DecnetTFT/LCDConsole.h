#ifndef LCDCONSOLE_H
#define LCDCONSOLE_H

#include <TFT.h>

class LCDConsole {
  public: 
    LCDConsole();
    bool begin(TFT &);
    bool println(String &msg);
    bool println(const char *);

  private:
    int row;
    int col;
    TFT *theTFT;
};

#endif


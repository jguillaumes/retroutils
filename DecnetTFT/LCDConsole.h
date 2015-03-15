/////////////////////////////////////////////////////////////////////////////
// DECNET Hello listener V01.00                                            //
/////////////////////////////////////////////////////////////////////////////
// Dumb console class definition
//
#ifndef LCDCONSOLE_H
#define LCDCONSOLE_H

#include <TFT.h>
#include <avr/pgmspace.h>

class LCDConsole {
  public: 
    LCDConsole();
    bool begin(TFT &);              // Initialize "console"
    bool println(String &msg);      // Output message, C++ String
    bool println(const char *);     // Output message, C string
    bool printmem(int);             // Output message, FLASH table entry
  private:
    int row;                        // Current row (text coordinate)
    TFT *theTFT;                    // Pointer to the real TFT
};

#endif


/********************************************************************************
Copyright (c) 2012, Jordi Guillaumes i Pons
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

***********************************************************************/


#include <stdlib.h>
#include <string.h>
#include "bin2load.h"



static int bigEndian = 0;       // Running system is bigendian
static const int K1 = 1;        // Constant to check endianness

/*
** Check running system endianness and save in static variable
*/
void initConversions() {
    bigEndian = ( (*(char*)&K1) == 0 );
}

/*
** Check validity of an octal digit
** We are not assuming our encoding is ASCII, so we don't use range check
** (EBCDIC has "holes" in the encoding)
*/
static inline int isOctalDigit(char d) {
    int result = 1;

    switch(d) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            break;
        default:
            result = 0;
    }
    return result;
}

/*
** Get the octal value of a character between '0' and '7'
** We are not assuming ASCII encoding, so we don't use the
** x - '0' trick.
*/
static inline int digitValue(char d) {
    int val = 0;

    switch(d) {
        case '0':
            val = 0;
            break;
        case '1':
            val = 1;
            break;
        case '2':
            val = 2;
            break;
        case '3':
            val = 3;
            break;
        case '4':
            val = 4;
            break;
        case '5':
            val = 5;
            break;
        case '6':
            val = 6;
            break;
        case '7':
            val = 7;
            break;
    }
    return val;
}

/*
** Convert any word (16 bit value) to little-endian representation
*/
void word2array(WORD word, BYTE array[2]) {
    BYTE *original = (BYTE *) &word;
    if (bigEndian) {
        array[0] = original[1];
        array[1] = original[0];
    } else {
        array[0] = original[0];
        array[1] = original[1];
    }
}




/*
** Get a word in little-endian format
*/
void array2word(BYTE array[2], WORD *word) {
    BYTE *theWord = (BYTE *) word;
    if (bigEndian) {
        theWord[0] = array[1];
        theWord[1] = array[0];
    } else {
        theWord[0] = array[0];
        theWord[1] = array[1];
    }
}

/*
** Verify the validity of an octal string
*/
int checkOctalString(char const * octalString) {
    int i=0, siz=0;
    char digit = 0;

    siz = strlen(octalString);
    for(i=0; i<siz; i++) {
        digit = octalString[i];
        if (!isOctalDigit(digit)) {
            return -1;
        }
    }
    return 0;
}

/*
** Convert an octal string to integer value
** This function stops converting either at the end of the input string
** or if it finds a non-octal digit.
*/
int octalValue(char const *octalString) {
    int numberValue = 0;
    int i=0, siz=0;
    char digit = 0;

    siz = strlen(octalString);

    for(i=0; i<siz; i++) {
        digit = octalString[i];
        if (isOctalDigit(digit)) {
            numberValue <<= 3;      // numberValue = numberValue * 8
            numberValue += digitValue(digit);
        } else {
            break;
        }
    }
    return numberValue;
}


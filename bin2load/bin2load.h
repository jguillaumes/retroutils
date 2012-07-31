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

#ifndef BIN2LOAD_H_INCLUDED
#define BIN2LOAD_H_INCLUDED

#define SECT_ALIGN 0100
#define BIN2LOAD_VERSION "1.00"

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long   LONGWORD;

struct BLOCK_HEADER_S {
    BYTE    k1;
    BYTE    filler;
    BYTE    count[2];
    BYTE    origin[2];
};

typedef struct BLOCK_HEADER_S BLOCK_HEADER;

int loadBinary(char const *fileName, BYTE **binBlob, int *binSize);
int saveLdaFromBin(char const *fileName, BYTE *binBlob, WORD const binSize, WORD const loadBase, WORD const startAddr);
int saveLdaFromAout(char const *fileName, BYTE *binBlob, WORD const binSize, BYTE padChar);


void initConversions();
void word2array(WORD word, BYTE array[2]);
void array2word(BYTE array[2], WORD *word);
int checkOctalString(char const *octalString);
int octalValue(char const *octalString);

#endif // BIN2LOAD_H_INCLUDED

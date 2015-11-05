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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "bin2load.h"

int verbose=0;      // Global variable to enable verbose output

/*
** Syntax help
*/
void syntax(char const *msg) {
    fprintf(stderr,"%s\n",msg);
    fprintf(stderr,"Usage: bin2load [-a] -f inputFile -o outputFile [-b load address (octal)] \n");
    exit(128);
}

/*
** Main procedure
*/
int main(int argc, char **argv)
{
    char msgBuffer[128];            // Work space for error messages
    int c;                          // Parsed option
    char *octAddr       = NULL;     // Parameter: octal base address
    char *inputBinFile  = NULL;     // Parameter: input file name
    char *outputLdaFile = NULL;     // Parameter: output file name
    BYTE *binBlob = NULL;           // Address of the buffer wirh loaded binary data
    int  binSize = 0;               // Size of the binary file (in bytes)
    int  loadAddr = 01000;          // Base address, as an integer (Defaults to 1000 oct)
    int  aout = 0;                  // Flag: input in a.out format

    if (argc == 1) {
        syntax("Usage:");
    }
    initConversions();      // Init endianness

    /*
    ** Parse the command line using the getopt() POSIX call
    ** Note this is not the "extended" GNU version
    */
    while((c=getopt(argc, argv, "avb:f:o:"))!=-1) {
        switch(c) {
            case 'a':
                aout = 1;
                break;
            case 'v':
                verbose = 1;
                break;
            case 'b':
                octAddr = optarg;
                break;
            case 'f':
                inputBinFile = optarg;
                break;
            case 'o':
                outputLdaFile = optarg;
                break;
            case '?':
                sprintf(msgBuffer, "Unknown option: %c", c);
                syntax(msgBuffer);
                break;
            default:
                syntax("This should not happen");
                break;
        }
    }

    /*
    ** Check for presence of parameters
    */
    if (inputBinFile == NULL || outputLdaFile == NULL) {
        syntax("Missing information");
    }

    /*
    ** Check octal value for base address
    */
    if (octAddr != NULL) {
        if (checkOctalString(octAddr) != 0) {
            fprintf(stderr,"Wrong octal value (%s)\n", octAddr);
            return(8);
        } else {
            loadAddr = octalValue(octAddr);
            if (verbose) {
                printf("Load address is %08o (0x%04X)\n", loadAddr, loadAddr);
            }
        }
    }

    /*
    ** Load the binary file into a buffer. The loadBinary() function
    ** takes care of allocating the buffer Notice that we pass the buffer address
    ** by reference (we pass the address of the pointer).
    */
    if (loadBinary(inputBinFile, &binBlob, &binSize) < 0) {
        fprintf(stderr, "Error loading binary file.\n");
        exit(16);
    }

    if (!aout) {
        if (saveLdaFromBin(outputLdaFile, binBlob, binSize, loadAddr) < 0) {
            fprintf(stderr, "Error saving load file.\n");
            exit(16);
        }
    } else {
        if (saveLdaFromAout(outputLdaFile, binBlob, binSize) < 0) {
            fprintf(stderr, "Error saving load file.\n");
            exit(16);
        }
    }

    return 0;
}

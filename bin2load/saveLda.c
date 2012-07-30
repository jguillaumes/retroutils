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
#include <memory.h>
#include "bin2load.h"

/**
** Save a LDA block, generating the header and computing the checksum
**/
int saveBlock(FILE *outFile, BYTE *binBlob, int const binSize, int const loadBase) {
    BLOCK_HEADER bh;
    WORD lb = 0;            // Load address
    WORD sz = 0;            // Block size
    int i=0;
    int check = 0;          // Running total for the checksum
    char ccheck = 0;        // Lowest byte of the checksum
    BYTE *bytePtr = NULL;

    lb = loadBase;
    sz = binSize + 6;

    bh.k1       = 1;        // Constant values for first 2 bytes
    bh.filler   = 0;        // of the block header

    word2array(lb, bh.origin);  // Move the load address in little-endian order
    word2array(sz, bh.count);   // Move the size in little-endian order

    /*
    ** Write the header
    */
    if (fwrite(&bh, sizeof(bh), 1, outFile) != 1) {
        perror("");
        fprintf(stderr,"Can't write a block header\n");
        return -1;
    }

    /*
    ** Add the header bytes to the checksum
    */
    bytePtr = (BYTE *) &bh;
    for(i=0; i<6; i++) {
        check += *bytePtr++;
    }

    /*
    ** Write the block data computing the checksum
    */
    bytePtr = binBlob;
    for (i=0; i<binSize; i++) {
        check += *bytePtr;
        if (fwrite(bytePtr, 1, 1, outFile) != 1) {
            perror("");
            fprintf(stderr,"Can't write to the output file");
            return -1;
        }
        bytePtr++;
    }
    ccheck = (char) check & 0377;   // Final checksum. Get the lowest order byte...
    ccheck = 0 - ccheck;            // ... and negate it
    if (fwrite(&ccheck, 1, 1, outFile) != 1) {  // Write the checksum byte
        perror("");
        fprintf(stderr,"Can't write to the output file");
        return -1;
    }
    return 0;
}

/*
* Create the LDA from an a.out (PDP11 style) file.
*/
int saveLdaFromAout(char const *fileName, BYTE *aoutBlob, int const aoutSize) {
    int rc=0;
    struct pdp11_external_exec *execHdr;    // PDP-11 a.out header
    BYTE *textPtr = NULL;                   // Pointer to text section in aoutBlob
    BYTE *dataPtr = NULL;                   // Pointer to data section in aoutBlob
    BYTE *bssPtr = NULL;                    // Pointer to bss section (to be allocated)
    WORD textSize = 0;                      // Text section size
    WORD dataSize = 0;                      // Data section size
    WORD bssSize = 0;                       // Bss section size
    WORD entryPoint = 0;                    // Code entry point
    int dataLoadAddr = 0;                   // Data load address (in the LDA file)
    int bssLoadAddr = 0;                    // BSS load address (in the LDA file)
    FILE *outFile = NULL;

    execHdr = (struct pdp11_external_exec *) aoutBlob;  // The header is at the beginning

    textPtr = aoutBlob + sizeof(*execHdr) - 1;  // Start of the text section...
    array2word(execHdr->e_text, &textSize);     // Length of the text section
    dataPtr = textPtr + textSize;               // Data: after the text
    array2word(execHdr->e_data, &dataSize);     // Data size, from array to word...
    array2word(execHdr->e_bss, &bssSize);       // Bss size, from array to word...
    array2word(execHdr->e_entry, &entryPoint);  // Entrypoint, from array to word...

    /*
    ** If we have a bss section we will pre-allocate it
    */
    if (bssSize > 0) {
        if ((bssPtr = malloc(bssSize)) == NULL) {
            perror("");
            fprintf(stderr,"Unable to allocate %d bytes of memory\n", bssSize);
            return(-1);
        } else {
            memset(bssPtr, 'U', bssSize);   // U for Uninitialized, really needed?
        }
    } else {
        bssPtr = NULL;
    }

    outFile = fopen(fileName,"w");

    if (outFile == NULL) {
        perror("opening output file");
        fprintf(stderr, "Can't open the output file (%s)\n", fileName);
        return -1;
    }

    /*
    ** First LDA bloc: text (code) section
    */
    rc = saveBlock(outFile, textPtr, textSize, entryPoint);
    if (rc == 0) {
        /*
        ** If we have a data section, we align it to a boundary and
        ** save it in a new block.
        ** In this version the boundary is fixed (SECT_ALIGN, defined in bin2load.h)
        ** I'll give the chance of specifying it as a program argument... perhaps
        **
        ** WARNING: The boundary size MUST be the same as the specified in the link
        ** script, else BAD THINGS WILL HAPPEN...
        */
        if (dataSize > 0) {
            dataLoadAddr = entryPoint + textSize;
            dataLoadAddr += SECT_ALIGN - 1;
            dataLoadAddr -= (dataLoadAddr % SECT_ALIGN);
            rc = saveBlock(outFile, dataPtr, dataSize, dataLoadAddr);
        }
    }
    /*
    ** If we have a bss section, we generate a block for it and align the
    ** beginning to SECT_ALIGN boundary
    */
    if (rc==0) {
        if (bssSize > 0) {
            bssLoadAddr = dataLoadAddr + dataSize;
            bssLoadAddr = (bssLoadAddr + SECT_ALIGN) - (bssLoadAddr % SECT_ALIGN);
            rc = saveBlock(outFile, bssPtr, bssSize, bssLoadAddr);
            free(bssPtr);
        }
    }
    if (rc == 0) rc = saveBlock(outFile, NULL, 0, entryPoint);
    return rc;
}

/*
** Create an LDA file from a binary, unstructured blob
*/
int saveLdaFromBin(char const *fileName, BYTE *binBlob, int const binSize, int const loadBase) {
    FILE *outFile = NULL;
    outFile = fopen(fileName,"w");
    int rc = 0;

    if (outFile == NULL) {
        perror("opening output file");
        fprintf(stderr, "Can't open the output file (%s)\n", fileName);
        return -1;
    }

    /*
    ** Save the whole data in one block
    ** Obviously it would be necessary to do some boundary checking
    ** Perhaps in the next version?
    */
    rc = saveBlock(outFile, binBlob, binSize, loadBase);
    if (rc == 0) rc = saveBlock(outFile, NULL, 0, loadBase);
    fclose(outFile);
    return(rc);
}



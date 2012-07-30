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
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "bin2load.h"

extern int verbose;

/*
** Read the contents of any file and return its contsnts in a buffer.
** This funcion allocates the space for the buffer
*/
int loadBinary(char const *fileName, BYTE **binBlob, int *binSize) {
    FILE *binFile = NULL;       // Stream pointer
    BYTE *buffer = NULL;        // Buffer (to allocate)
    int fileDesc = 0;           // File descriptor (for fstat)
    struct stat fileStat;       // File status structure
    int bytesRead = 0;          // Actual bytes read

    binFile = fopen(fileName,"r");
    if (binFile == NULL) {
        perror("opening binary file");
        fprintf(stderr,"Can't open file %s\n", fileName);
        return -1;
    }

    /*
    ** Get file size using the fstat() POSIX call
    */
    fileDesc = fileno(binFile);
    if (fstat(fileDesc, &fileStat) < 0) {
        perror("getting file information");
        fprintf(stderr,"Can't get size of file %s\n", fileName);
        return -1;
    }
    if (verbose) {
        printf("Size of %s (input file): %d (decimal)\n", fileName, (int) fileStat.st_size);
    }

    /*
    ** Allocate a big enough buffer to read the whole file
    */
    buffer = malloc(fileStat.st_size);
    if (buffer == NULL) {
        perror("allocating buffer");
        fprintf(stderr, "Can't allocate buffer of %d bytes\n", (int) fileStat.st_size);
        return -1;
    }

    /*
    ** Read the whole file in one fread() call
    */
    bytesRead = fread(buffer, 1, fileStat.st_size, binFile);
    if (bytesRead != fileStat.st_size) {
        perror("");
        fprintf(stderr,"Error reading input file\n");
        return(-1);
    }

    /*
    ** Move buffer address and size to input parameters and close the file
    */
    *binSize = fileStat.st_size;
    fclose(binFile);
    *binBlob = buffer;
    return(0);
}

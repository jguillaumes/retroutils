/* 
 * File:   DecnetFileSaver.cpp
 * Author: jguillaumes
 * 
 * Created on 28 / gener / 2015, 23:17
 */
#include <ios>
#include <string>
#include <fstream>
#include <iostream>

#include <sys/time.h>
#include <errno.h>
#include <string.h>

#include "FileSaver.h"

FileSaver::FileSaver(const std::string &fileName) {
    static const int   magic = 0xa1b2c3d4;  /* Magic number */
    static const short major = 0x02;        /* Version (major) */
    static const short minor = 0x04;        /* Version (minor) */
    static const int   thiszone = 0;        /* Timezone adjustment: none */
    static const int   sigfigs  = 0;        /* */
    static const int   snaplen  = 65535;    /* Maximum packet length */
    static const int   network  = 1;         /* LINKTYPE_ETHERNET */
    
    fileOpened = false;
    if (fileName.length() == 0) return;
    
    capfile.rdbuf()->pubsetbuf(0,0);
    capfile.open(fileName.c_str(), ios::out | ios::binary);
    if (capfile.is_open()) {
        fileOpened = true;
        capfile.write((char *)&magic, 4);
        capfile.write((char *)&major, 2);
        capfile.write((char *)&minor, 2);
        capfile.write((char *)&thiszone, 4);
        capfile.write((char *)&sigfigs, 4);
        capfile.write((char *)&snaplen, 4);
        capfile.write((char *)&network, 4);
        capfile.flush();
    } else {
        msgError.assign((const char *)strerror(errno));
    }
}

FileSaver::~FileSaver() {
   if (capfile.is_open()) {
        capfile.close();
        fileOpened = false;
   }
}

/*
 * Write a captured packet, with its header
 */
void FileSaver::savePacket(const BYTE* packet, int len) {
    int time;               /* Timestamp (seconds, standard unix) */
    int usec;               /* Microseconds (since last second) */
    int incl_len;           /* Captured length */
    int orig_len;           /* Real length */
    struct timeval tv;  
    struct timezone tz;
    gettimeofday(&tv, NULL);
    
    time = tv.tv_sec;
    usec = tv.tv_usec;
    incl_len = len;         /* We always capture the full packet */
    orig_len = len;
    
    capfile.write((char *)&time, 4);
    capfile.write((char *)&usec, 4);
    capfile.write((char *)&incl_len, 4);
    capfile.write((char *)&orig_len, 4);
    capfile.write((char *)packet, len);
}
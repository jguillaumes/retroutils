/* 
 * File:   main.cpp
 * Author: jguillaumes
 *
 * Created on 28 / gener / 2015, 12:07
 */

#include <getopt.h>
#include <errno.h>

#include "DecnetDefs.h"
#include "DecnetListenerPlus.h"
#include "DecnetListener.h"
#include "DecnetListenerPcap.h"

using namespace std;



/*
 * Basic syntax help
 */
void help() {
    fprintf(stderr, "Usage: DecnetListener -i interface [-f capture_filename]\n");
}


/*
 * 
 */
int main(int argc, char** argv) {
    static char opts[] = "+i:f:a";
    std::string iface; 
    std::string fileName;
    char c;
    bool saveAll;
    
     /*
     * Parameter parsing loop.
     * It uses libgetopt, without gnu extensions.
     */
    saveAll = false;
    while ((c = getopt(argc, argv, opts)) != -1) {
        switch (c) {
            case 'i':
                iface.assign(optarg);
                break;
            case 'f':
                fileName.assign(optarg);            
                break;
            case 'a':
                saveAll = true;
                break;
            default:
                help();
                exit(16);
        }
    }

    /*
     * The -i parameter is mandatory
     */
    if (iface.length() == 0) {
        help();
        exit(32);
    }    
    
    if (saveAll && (fileName.length() == 0)) {
        cout << "'-a' requires '-f'" << endl;
        exit(32);
    }
    
    DecnetListenerPcap *dl = new DecnetListenerPcap(iface, fileName);
    if (dl->isCapturing()) {
        dl->setSaveAll(saveAll);
        dl->captureLoop();
    } else {
        cout << dl->getMsgError() << endl;
    }
    delete dl;
    return 0;
}

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

const std::string DecnetListenerPcap::capFilter = "ether host AB-00-00-03-00-00";


/*
 * Basic syntax help
 */
void help() {
    fprintf(stderr, "Usage: DecnetListener -i interface [-f capture_filename]\n");
}

/*
 *  Translate the routing message type code to its name.
 */
const std::string typeName(unsigned int nodeType) {
   std::string type;
    
    switch (nodeType) {
        case 1:
            type = "LEVEL 2 ROUTER";
            break;

        case 2:
            type = "LEVEL 1 ROUTER";
            break;

        case 3:
            type = "END NODE";
            break;

        default:
            type = "*UNKNOWN*";
    }

    return type;
}

void tractaHello(struct hello_t *hello) {
    std::string type;

    type = typeName(hello -> nodeInfo.nodeType);
    std::cout << "HELLO packet from ";
    std::cout.width(2);
    std::cout.setf(std::ios::right, std::ios::adjustfield);
    std::cout << AREA(hello -> dnAddr) << ".";
    std::cout.width(4);
    std::cout.setf(std::ios::left, std::ios::adjustfield); 
    std::cout << NODE(hello -> dnAddr) << ", type=" ;
    std::cout.width(14);
    std::cout << type 
              << "(" << hello->nodeInfo.nodeType << ")" 
              << ", rtype=(" << hello->routingFlags.type << ")";
    if (hello -> nodeInfo.nodeType == 3) {
        std::cout << " Timer: " << hello->u.endNode.helloTimer;
        // printf(" Timer: %d DR: %d.%d\n", hello->u.endNode.helloTimer,AREA(dnFromMac(hello -> u.endNode.designatedRouter)), NODE(dnFromMac(hello -> u.endNode.designatedRouter)));
    } else {
        std::cout << " Timer: " << hello->u.router.helloTimer; 
        // printf(" Timer: %d Prio: %d\n", hello->u.router.helloTimer, hello->u.router.priority);
    }
    cout << endl;
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
        dl->captureLoop(tractaHello);
    } else {
        cout << dl->getMsgError() << endl;
    }
    delete dl;
    return 0;
}

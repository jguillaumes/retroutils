/*
 * DECNET hello sniffer
 * 
 * This program logs on stdout all the DECNET hello frames
 * transmitted in the local ethernet. Optionally, it captures
 * "unknown" frames (ie, frames unknown to the program) in a file
 * in a format suitable to be interpreted by wireshark or tcpdump.
 * 
 * File:   main.c
 * Author: jguillaumes
 *
 * Created on 24 / gener / 2015, 20:38
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/time.h>

#include "DecnetListener.h"
#include "commands.h"

#define DEBUG 0

FILE *capture;          /* Packet capture file */
int cont;               /* Continue flag       */
static int invokeCommands = 0;
extern FILE *yyin;
sigjmp_buf env;

/*
 * Basic syntax help
 */
void help() {
    fprintf(stderr, "Usage: DecnetListener -i interface [-f capture_filename]\n");
}

/*
 * Control-C handler: if there is a capture file active,
 * it closes it.
 */
void intHandler(int sig) {
    signal(sig, SIG_IGN);
    invokeCommands = 1;
    siglongjmp(env,0);
}

int main(int argc, char ** argv) {
    char * iface = NULL;
    int i, j;
    int c;
    static char opts[] = "+i:f:";
    static char filter[] = "ether host AB-00-00-03-00-00";
    pcap_t * handle;
    struct bpf_program pf;
    struct pcap_pkthdr header;
    BYTE * packet;
    struct frame_s *frame;
    WORD * etherType;
    WORD * plSize;
    struct hello_t *hello;
    char error[512];
    
    /*
     * Parameter parsing loop.
     * It uses libgetopt, without gnu extensions.
     */
    while ((c = getopt(argc, argv, opts)) != -1) {
        switch (c) {
            case 'i':
                iface = optarg;
                break;
            case 'f':
                capture = openPacketFile(optarg);
                if (capture == NULL) {
                    perror("Capture file open:");
                    exit(128);
                }
                break;
            default:
                help();
                exit(16);
        }
    }

    /*
     * The -i parameter is mandatory
     */
    if (iface == NULL) {
        help();
        exit(32);
    }

    /*
     * Open the packet capture (libpcap) and prepare the capture
     * filter. The capture filter specifies to get all the traffic
     * sent to the DECNET routing multicast address.
     */
    handle = pcap_open_live(iface, 512, 1, 0, error);

    if (handle == NULL) {
        fprintf(stderr, "Error opening device %s: %s\n", iface, error);
        exit(64);
    }

    if (-1 == pcap_compile(handle, &pf, filter, 1, 0)) {
        fprintf(stderr, "Error compiling the filter expression (%s)\n", pcap_geterr(handle));
        exit(64);
    }

    if (-1 == pcap_setfilter(handle, &pf)) {
        fprintf(stderr, "Error setting up filter (%s)\n", pcap_geterr(handle));
        exit(64);
    }

    /*
     * Establish CTRL-C handler
     */
    signal(SIGINT,intHandler);
    
    /*
     * Infinite loop, breakable using CTRL-C
     */
    cont = 1;
loop:
    while (cont != 0) {
        hello = NULL;
        frame = NULL;

        sigsetjmp(env,1);
        
        if (invokeCommands == 1) {
            printf("\nSuspending process...\n");

            cont = parseCommands(stdin,stdout);

            if (cont != 0) {
                invokeCommands = 0;
                signal(SIGINT,intHandler);
                printf("\nResuming process...\n");
            } else {
                printf("\nFinishing process...\n");
                break;
            }
        }

        /*
         * Capture a packet and establish pointer to the captured frame
         */
        packet = pcap_next(handle, &header);
        frame = (struct frame_s*) packet;

#if (DEBUG == 1)
        printf("Src: %d.%d | ", AREA(dnFromMac(frame -> dst)), NODE(dnFromMac(frame -> dst)));
#endif
        /*
         * Establish pointer to (possible) ethertype
         */
        etherType = ((void *) packet) + OFS_ETHERTYPE;
#if (DEBUG == 1)
        printf("\neth=%x(%04x)\n", etherType,*etherType);
#endif
        /*
         * If this is a tagged frame (VLAN), then the real ethertype
         * is just after the VLAN tag
         */
        if (*etherType == ET_VLANTAG) {
            etherType += 1;
        }

        /*
         * Check for DECNET routing frame
         */
        if (*etherType == ET_DNETROUTING) {
            /*
             * Skip the payload size
             */
            plSize = ((void *) etherType) + OFS_FRAMESIZE;
            hello = ((void *) plSize) + OFS_FRAME;
#if (DEBUG == 1)
            printf("pls=%x(%d/%04x)\n", plSize, *plSize, *plSize);
            printf("first byte=%02x\n", *(BYTE *)hello);
#endif
            /*
             * If the first byte of the payload has its high bit set
             * then it is a padding counter, so we skip it.
             */
            if ((*((BYTE *)hello) & 0x80) == 0x80) {
                hello = ((void *)hello) + 1;
            }
        }
#if (DEBUG == 1)
        printf("hello=%x\n", hello);
#endif
        /*
         * Now, if hello is not null then it points to the hello
         * message and we can handle it.
         */
        if (hello != NULL) {
            switch (hello -> routingFlags.type) {
                case 6:         /* Endnode Hello */
                case 5:         /* Router Hello  */
                    processHello(hello);
                    break;
                case 0:         /* Route init message */
                    processInit((struct init_t*) hello);
                    break;
                case 4:         /* Level 2 routing message */
                case 3:         /* Level 1 routing message */
                    break;
                default:        /* Unknown message: log it if requested */
                    if (capture != NULL) {
                        writePacket(capture, header.len, packet);
                    }
                    printf("\n");
                    break;
            }
        }
    }
    if (capture != NULL) {
        printf("Closing capture file...\n");
        fclose(capture);
    }
    return (EXIT_SUCCESS);
}

/*
 *  Process (display) a hello message
 */
void processHello(struct hello_t *hello) {
    char type[64];

    typeName(hello -> nodeInfo.nodeType, type);
    printf("HELLO packet from %2d.%-4d, type=%-14s(%d), rtype=(%d) ", AREA(hello -> dnAddr), NODE(hello -> dnAddr), type,
            hello -> nodeInfo.nodeType, hello -> routingFlags.type);

    
    if (hello -> nodeInfo.nodeType == 3) {
        printf(" Timer: %d DR: %d.%d\n", hello->u.endNode.helloTimer,AREA(dnFromMac(hello -> u.endNode.designatedRouter)), NODE(dnFromMac(hello -> u.endNode.designatedRouter)));
    } else {
        printf(" Timer: %d Prio: %d\n", hello->u.router.helloTimer, hello->u.router.priority);
    }
}

/*
 *  Process (display) an init message
 */
void processInit(struct init_t *init) {
    char type[64];

    typeName(init -> nodeInfo.nodeType, type);
    printf("INIT packet from %d.%d, type=%s, timer=%d\n", AREA(init -> srcNode), NODE(init -> srcNode), type,
            init -> timer);
}

/*
 *  Extrat the DECNET address from a MAC address
 *  Just put the last two bytes, reverses, into a word.
 */
WORD dnFromMac(BYTE * mac) {
    WORD dnadr = 0;

    dnadr = (BYTE) mac[4];
    dnadr += 256 * (BYTE) mac[5];

    return dnadr;
}

/*
 *  Translate the routing message type code to its name.
 */
void typeName(unsigned int nodeType,
        char * outType) {
    char * type;

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

    strncpy(outType, type, 64);
}

/*
 * Open (create) the capture file and write the header
 */
FILE *openPacketFile(char *filename) {
    FILE *file;
    static const int   magic = 0xa1b2c3d4;  /* Magic number */
    static const short major = 0x02;        /* Version (major) */
    static const short minor = 0x04;        /* Version (minor) */
    static const int   thiszone = 0;        /* Timezone adjustment: none */
    static const int   sigfigs  = 0;        /* */
    static const int   snaplen  = 65535;    /* Maximum packet length */
    static const int   network  = 1;         /* LINKTYPE_ETHERNET */
    
    file = fopen(filename, "w");
    if (file != NULL) {
        fwrite(&magic, 4, 1, file);
        fwrite(&major, 2, 1, file);
        fwrite(&minor, 2, 1, file);
        fwrite(&thiszone, 4, 1, file);
        fwrite(&sigfigs, 4, 1, file);
        fwrite(&snaplen, 4, 1, file);
        fwrite(&network, 4, 1, file);
    }
    return file;
}

/*
 * Write a captured packet, with its header
 */
void writePacket(FILE *file, long len, BYTE *packet) {
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
    
    fwrite(&time, 4, 1, file);
    fwrite(&usec, 4, 1, file);
    fwrite(&incl_len, 4, 1, file);
    fwrite(&orig_len, 4, 1, file);
    fwrite(packet, len, 1, file);
}

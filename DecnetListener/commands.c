#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define PROMPT  "LSN>"
#define EXIT    "exit"
#define CONT    "cont"
#define SETM    "setm"
#define LIST    "list"
#define ENDL    "endl"
#define ZERO    "zero"
#define SMAC    "smac"
#define SNOD    "snod"
#define HELP    "help"

int parseCommands(FILE *cmds, FILE *out) {
    int exitLoop = 0;
    int retVal = 1;
    char line[81];
    char cmnd[5];
    char *p;
    char *a;

    while (exitLoop != 1) {
        fprintf(out, "%s ", PROMPT);
        memset(line,0,81);
        fgets(line, 81, cmds);
        for (p = line; *p; ++p) {
            *p = tolower(*p);
            if (*p == '\n') *p = 0;
        }

        p = strtok(line, " ");
        a = strtok(NULL, " ");

        if (p != NULL) {

            // fprintf(out, "p=[%s], a=[%s]\n", p, a);
            
            if (strlen(p) != 4) {
                fprintf(out, "Bad command: %s\n", p);
            } else {
                if (strcmp(p, CONT) == 0) {
                    retVal = 1;
                    exitLoop = 1;
                } else if (strcmp(p, EXIT) == 0) {
                    retVal = 0;
                    exitLoop = 1;
                } else if (strcmp(p, HELP) == 0) {
                    fprintf(out, "Listener commands:\n");
                    fprintf(out, "EXIT        - end process\n");
                    fprintf(out, "CONT        - resume process\n");
                    fprintf(out, "SMAC        - display MAC address\n");
                    fprintf(out, "SNOD XX.XXX - show node information\n");
                    fprintf(out, "SMAC        - display MAC address\n");
                    fprintf(out, "SETM xx...  - set local MAC address\n");
                    fprintf(out, "LIST        - enter list definition mode\n");
                    fprintf(out, "SMAC        - display MAC address\n");
                } else {
                    fprintf(out, "Command not implemented: %s\n", p);
                }
            }
        }
    }
    return retVal;
}

/**************************************************
 * FILENAME:        main.cpp
 * PROJECT:         BackItUp
 *
 * AUTHOR:          Philipp Doblhofer
 * WEB:             www.gnp-tec.net
 * START DATE:      2013-Dec-29
 *
 **************************************************
 * DESCRIPTION:
 * Main program, which initiates the process of
 * the backup. (Read config, start copying, ...)
 *************************************************/

#include <stdio.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include "../inc/config.h"
#include "../inc/fiterator.h"

#define ERR(x...)   {fprintf(stderr, x);}

#ifdef DEBUG
#define V_TYPE "- DEBUG"
#elif defined(PROFILING)
#define V_TYPE "- PROFILING"
#else
#define V_TYPE ""
#endif

void usage(const char* cmd) {
    ERR("usage: %s <command> [config-file]\n\r\n\r", cmd);
    ERR("Commands:\n\r");
    ERR("\tbackup\n\r");
    ERR("\trestore\n\r");
    ERR("\tverify\n\r");
    ERR("\tversion\n\r");
}

int main(int argc, char** argv) {
    if(argc == 1)
        usage(argv[0]);
    else {
        if(strncmp(argv[1], "backup", strlen("backup")+1) == 0) {
            if(argc < 3) {
                ERR("No config file is passed!\n\r\n\rusage: %s backup [config-file]\n\r", argv[0]);
                return 1;
            }
            Config c;
            if(!c.load(argv[2]) || !c.isValid())
                return 1;

            c.backupDirectories();
        } else if(strncmp(argv[1], "version", strlen("version")+1) == 0) {
            printf("BackItUp v%i.%i %s\n\rCopyright (c) 2014 GNP-Tec.net, Philipp Doblhofer\n\r", V_MAJ, V_MIN, V_TYPE);
            return 0;
        } else {
            ERR("Unknown command <%s>\n\r\n\r", argv[1]);
            usage(argv[0]);
            return 1;
        }
    }
    
    return 0;
#if 0
    Config c;
    if(argc < 2) {
        ERR("No arguments passed!\n\r");
        return 1;
    }

    if(!c.load(argv[1]))
        return 1;
    c.backupDirectories();
    return 0;
#endif
}

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

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include "../inc/config.h"
#include "../inc/fiterator.h"

#define ERR(x...)   {fprintf(stderr, x);}

int main(int argc, char** argv) {
    //backup("t", "backup.bkp");

    //readConfig(argv[1]);
    Config c;
    if(argc < 2) {
        ERR("No arguments passed!\n\r");
        return 1;
    }

    if(!c.load(argv[1]))
        return 1;
    c.backupDirectories();
    return 0;
}

/**************************************************
 * FILENAME:        BackItUp.h
 * PROJECT:         BackItUp
 *
 * AUTHOR:          Philipp Doblhofer
 * WEB:             www.gnp-tec.net
 * START DATE:      2014-Jan-18
 *
 **************************************************
 * DESCRIPTION:
 * "Glue-logic" class.
 *************************************************/

#ifndef _BACKITUP_H_
#define _BACKITUP_H_

#include "../inc/config.h"
#include <Logger.h>

extern Logger Log;

class Config;

class BackItUp {
    private:
        void usage(char* pgm);
        void version();
    
        char* configPath;
    public:
        Config c;

        BackItUp(int argc, char** argv);
        const char* GetConfigFile() { return configPath; }
};

#endif

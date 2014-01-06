/**************************************************
 * FILENAME:        filehandler.h
 * PROJECT:         BackItUp
 *
 * AUTHOR:          Philipp Doblhofer
 * WEB:             www.gnp-tec.net
 * START DATE:      2014-Jan-4
 *
 **************************************************
 * DESCRIPTION:
 * Simplifies the access to regular files and 
 * archives
 *************************************************/

#ifndef _FILEHANDLER_H_
#define _FILEHANDLER_H_

#include <string.h>
#include <stdlib.h>
#include <archive.h>
#include <sys/stat.h>
#include "../inc/config.h"

class Config;

class FileHandler {
    public:
        bool Init(Config *conf);

        bool copyDirectory(const char* src, const char* dest, struct stat *attr);  
        bool copyFile(const char* src, const char* dest, struct stat *attr);
        
        FileHandler() { c = NULL; }
        void Finalize();
    private:
        Config *c;
        struct archive *a;
};

#endif

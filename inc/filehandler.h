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
 * Simplifies the access to regular files, 
 * archives, ...
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
        virtual bool Init(Config *conf) { return false; };

        virtual bool copyDirectory(const char* src, const char* dest, struct stat *attr) { return false; };
        virtual bool copyFile(const char* src, const char* dest, struct stat *attr) { return false; };
        
        virtual void Finalize() {};

        FileHandler() { c = NULL; }
    protected:
        Config *c;
        struct archive *a;
};

#endif

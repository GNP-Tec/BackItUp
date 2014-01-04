/**************************************************
 * FILENAME:        config.h
 * PROJECT:         BackItUp
 *
 * AUTHOR:          Philipp Doblhofer
 * WEB:             www.gnp-tec.net
 * START DATE:      2013-Dec-29
 *
 **************************************************
 * DESCRIPTION:
 * This is the class, which reads the configuration
 * file for the backuper (XML). Afterwards it will
 * call the FIterator, which saves the files and
 * directories as set in the config file.
 *************************************************/

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "../inc/filehandler.h"
#include <string.h>

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include <Logger.h>

#include <vector>
using namespace std;

typedef enum bck_mode {
    MODE_UNSET=0,
    MODE_FULL
} BCK_MODE;


typedef enum bck_type {
    TYPE_UNSET=0,
    TYPE_UNCOMPRESSED,
    TYPE_COMPRESSED
} BCK_TYPE;

class FileHandler;

class Config {
    private:
        xmlDocPtr doc;
        char* configfile;
        char* configversion;

        void unload();
        void reset();

        char* backup_dest;
        BCK_MODE mode; 
        BCK_TYPE type;

        vector<const char*> directories;
    public:
        Logger log;
        FileHandler *FH;

        bool load(const char* file);
        void backupDirectories();

        const char* getBackupDestination() { return (const char*)backup_dest; }
    
        BCK_TYPE getType() { return type; }
    
        Config() {
            reset();
        }

        ~Config() {
            unload();
        }
};

#endif

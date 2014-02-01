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


#include <Logger.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include <list>
using namespace std;

extern Logger Log;

typedef enum _bkp_type {
    TYPE_UNSET, 
    TYPE_REGULAR,
    TYPE_COMPRESSED
#warning TYPE_ENCRYPTED
} BackupType;

class Config {
    private:
        char* version;
        char* destination;
        BackupType type;
        list<const char*> dir;
    public:
        Config() {
            version = NULL;
            destination = NULL;
            type = TYPE_UNSET;
        };

        ~Config() {
            if(version != NULL)
                free(version);
            if(destination != NULL)
                free(destination);
        };    

        bool Load(const char* cfg);
        bool Validate(void);

        const char* GetDestination() { return destination; };
        const char* GetVersion() { return version; };
        BackupType  GetBackupType() { return type; };

        const char* GetNextBackupDirectory() { const char* x = dir.front(); dir.pop_front(); return x; }
        bool        IsNextBackupDirectory() { return !dir.empty(); }
    
};



/*
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
        bool isValid();
        void backupDirectories();
        void verifyBackup();

        const char* getBackupDestination() { return (const char*)backup_dest; }
    
        BCK_TYPE getType() { return type; }
    
        Config() {
            reset();
        }

        ~Config() {
            unload();
        }
};*/

#endif

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <string.h>

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include <vector>
using namespace std;

typedef enum bck_mode {
    MODE_UNSET=0,
    MODE_FULL
} BCK_MODE;

class Config {
    private:
        xmlDocPtr doc;
        char* configfile;
        char* configversion;

        void unload();
        void reset();

        char* backup_dest;
        BCK_MODE mode;

        vector<const char*> directories;
    public:
        bool load(const char* file);
        void backupDirectories();

        Config() {
            reset();
        }

        ~Config() {
            unload();
        }
};

#endif

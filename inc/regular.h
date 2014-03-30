/**************************************************
 * FILENAME:        regular.h
 * PROJECT:         BackItUp
 *
 * AUTHOR:          Philipp Doblhofer
 * WEB:             www.gnp-tec.net
 * START DATE:      2014-Jan-25
 *
 **************************************************
 * DESCRIPTION:
 * The regular backup handler
 *************************************************/

#ifndef _REGULAR_H_
#define _REGULAR_H_

#include "../inc/Backup.h"
#include "../inc/BackItUp.h"
#include "../inc/FileTree.h"

#include <stdio.h>

class RegularBackup : public Backup {
    private:
        char *root_dir;
        FileTree ft;
    
        bool copyFile(const char* src, const char* dest, bool copy=true);
    public: 
        RegularBackup(BackItUp *ptr) : Backup(ptr) {
        }

        bool OpenBackup(const char* path);
        bool PrintConfig();
        FileTree GetFileTree();
        char* GetConfig();
        bool Compare();
        bool CloseBackup();
       
        bool Initialize();
        bool Finalize();
        bool addFolder(const char* path, bool init=true, bool copy=true);
};

#endif

/**************************************************
 * FILENAME:        compressed.h
 * PROJECT:         BackItUp
 *
 * AUTHOR:          Philipp Doblhofer
 * WEB:             www.gnp-tec.net
 * START DATE:      2014-Feb-01
 *
 **************************************************
 * DESCRIPTION:
 * The compressed backup handler (.tar.gz)
 *************************************************/

#ifndef _COMPRESSED_H_
#define _COMPRESSED_H_

#include "../inc/Backup.h"
#include "../inc/BackItUp.h"
#include "../inc/FileTree.h"

#include <stdio.h>
#include <archive.h>
#include <archive_entry.h>

class CompressedBackup : public Backup {
    private:
        char *root_file;
        struct archive *a;
        FileTree ft;
    
        bool copyFile(const char* src, const char* dest);
    public: 
        CompressedBackup(BackItUp *ptr) : Backup(ptr) {
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

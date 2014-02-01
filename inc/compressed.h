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

#include <stdio.h>
#include <archive.h>
#include <archive_entry.h>

class CompressedBackup : public Backup {
    private:
        char *root_file;
        struct archive *a;
    
        bool copyFile(const char* src, const char* dest);

    public: 
        CompressedBackup(BackItUp *ptr) : Backup(ptr) {
        }
       
        bool Initialize();
        bool Finalize();
        bool addFolder(const char* path, bool init=true);
        // getFileTree
        // compare
};

#endif

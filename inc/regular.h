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

#include <stdio.h>

class RegularBackup : public Backup {
    private:
        char *root_dir;
    
        bool copyFile(const char* src, const char* dest);

    public: 
        RegularBackup(BackItUp *ptr) : Backup(ptr) {
        }
       
        bool Initialize();
        bool Finalize();
        bool addFolder(const char* path, bool init=true);
        // getFileTree
        // compare
};

#endif

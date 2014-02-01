/**************************************************
 * FILENAME:        Backup.h
 * PROJECT:         BackItUp
 *
 * AUTHOR:          Philipp Doblhofer
 * WEB:             www.gnp-tec.net
 * START DATE:      2014-Jan-25
 *
 **************************************************
 * DESCRIPTION:
 * Interface-class to the backup-files
 *************************************************/

#ifndef _BACKUP_H_
#define _BACKUP_H_

#include "../inc/BackItUp.h"

class Backup {
    protected:
        BackItUp *b;
    public:
        Backup(BackItUp *ptr) { b = ptr; }    
    
        virtual bool Initialize(/*...*/) { return false; };
        virtual bool Finalize(/*...*/) { return false; };
        virtual bool addFolder(const char* path, bool init=true) { return false; };
        // getFileTree
        // compare
};

#endif

/**************************************************
 * FILENAME:        cfilehandler.h
 * PROJECT:         BackItUp
 *
 * AUTHOR:          Philipp Doblhofer
 * WEB:             www.gnp-tec.net
 * START DATE:      2014-Jan-17
 *
 **************************************************
 * DESCRIPTION:
 * Compressed file handler
 *************************************************/

#ifndef _CFILEHANDLER_H_
#define _CFILEHANDLER_H_

#include "../inc/filehandler.h"


class CFileHandler : public FileHandler {
    public:
        bool Init(Config *conf);

        bool copyDirectory(const char* src, const char* dest, struct stat *attr);  
        bool copyFile(const char* src, const char* dest, struct stat *attr);
        
        void Finalize();
};

#endif

/**************************************************
 * FILENAME:        fiterator.h
 * PROJECT:         BackItUp
 *
 * AUTHOR:          Philipp Doblhofer
 * WEB:             www.gnp-tec.net
 * START DATE:      2013-Dec-29
 *
 **************************************************
 * DESCRIPTION:
 * This is the class, which saves the files 
 * and directories. They have afterwards the same
 * permissions and modification date is set in
 * the destination file as the source file is.
 *************************************************/

#ifndef _FITERATOR_H_
#define _FITERATOR_H_

#include "../inc/config.h"

class FIterator {
    private:
#ifdef SIZE
        unsigned long long size;
#endif
    public:
        FIterator(Config *c, const char* directory, bool isRootDirectory=true);
#ifdef SIZE
        unsigned long long getSize() { return size; }
#endif
};

#endif

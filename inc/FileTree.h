/**************************************************
 * FILENAME:        FileTree.h
 * PROJECT:         BackItUp
 *
 * AUTHOR:          Philipp Doblhofer
 * WEB:             www.gnp-tec.net
 * START DATE:      2014-Feb-01
 *
 **************************************************
 * DESCRIPTION:
 * FileTree class
 *************************************************/

#ifndef _FILETREE_H_
#define _FILETREE_H_

#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct _FileTreeElement {
    char* Name;
    struct stat attr;

    struct _FileTreeElement *pNext;
    struct _FileTreeElement *pPrev;
} FileTreeElement;

class FileTree {
    private:
        FileTreeElement* pRoot;
    public:
        FileTree() { pRoot = NULL; }
        ~FileTree() { 
            reset();
        };

        bool addEntry(const char* name, struct stat attr);
        const char* serialize(FileTreeElement *pPtr);
        const char* getNextSerializedElement();
        unsigned long getSerializedSize();
        void reset();
        FileTreeElement* getNextElement(bool reset=false);
        FileTreeElement* FindByName(char* name);
        bool removeElement(FileTreeElement* ptr);
        bool compare(FileTree *o);
        void printChanges(FileTree *o);
        void print();
};

#endif

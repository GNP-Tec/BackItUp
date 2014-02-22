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
            while(pRoot != NULL) {
                free(pRoot->Name);
                if(pRoot->pNext == NULL) {
                    free(pRoot);
                    break;
                }
                pRoot = pRoot->pNext;
                free(pRoot->pPrev);
            }
        };

        bool addEntry(const char* name, struct stat attr);
        const char* serialize(FileTreeElement *pPtr);
        const char* getNextSerializedElement();
};

#endif

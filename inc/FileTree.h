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

        void reset() {
            while(pRoot != NULL) {
                free(pRoot->Name);
                if(pRoot->pNext == NULL) {
                    free(pRoot);
                    break;
                }
                pRoot = pRoot->pNext;
                free(pRoot->pPrev);
            }
            pRoot = NULL;
        }

        FileTreeElement* getNextElement() {
            static FileTreeElement *pTmp = pRoot;
            FileTreeElement *pTmp2 = pTmp;
            if(pTmp == NULL)
                return NULL;
            pTmp = pTmp->pNext;

            return pTmp2;            
        }

        FileTreeElement* FindByName(char* name) {
            FileTreeElement *pTmp = pRoot; 
            
            while(pTmp != NULL) {
                if(strcmp(name, pTmp->Name) == 0)
                    return pTmp;
                pTmp = pTmp->pNext;
            }
            return pTmp;
        }

        bool removeElement(FileTreeElement* ptr) {
            if(ptr == NULL)
                return false;

            if(ptr->pNext != NULL)
                ptr->pNext->pPrev = ptr->pPrev;
            if(ptr->pPrev != NULL)
                ptr->pPrev->pNext = ptr->pNext;

            if(ptr == pRoot) 
                pRoot = ptr->pNext;

            free(ptr->Name);
            free(ptr);
    
            return true;
        }

        bool compare(FileTree *o) {
            FileTreeElement* ptr_o;
            bool ret = true;

            while((ptr_o = o->getNextElement()) != NULL) {
                bool differs = false;
                FileTreeElement *ptr_l = FindByName(ptr_o->Name);

#warning deleted files
                if(ptr_l == NULL) {
                    ret = false;
                    differs = true;
                    printf("<%s> not found\n", ptr_o->Name);
                } else {
                    if(ptr_l->attr.st_uid != ptr_o->attr.st_uid) {
                        ret = false;
                        differs = true;
                        printf("<%s> uid differs\n", ptr_o->Name);
                    }
                    if(ptr_l->attr.st_gid != ptr_o->attr.st_gid) {
                        ret = false;
                        differs = true;
                        printf("<%s> gid differs\n", ptr_o->Name);
                    }
                    if(ptr_l->attr.st_mode != ptr_o->attr.st_mode) {
                        ret = false;
                        differs = true;
                        printf("<%s> mode differs\n", ptr_o->Name);
                    }
                    if(ptr_l->attr.st_size != ptr_o->attr.st_size) {
                        ret = false;
                        differs = true;
                        printf("<%s> size differs\n", ptr_o->Name);
                    } 
                    if(ptr_l->attr.st_mtime != ptr_o->attr.st_mtime) {
                        ret = false;
                        differs = true;
                        printf("<%s> modtime differs\n", ptr_o->Name);
                    }
                }

                if(!differs) {
                    removeElement(ptr_l);
                    o->removeElement(ptr_o);
                }
            }
    
            return ret;
        }

        void print() {
            FileTreeElement *ptr = pRoot;
            while(ptr != NULL) {
                printf("%s\n", ptr->Name);
                ptr = ptr->pNext;
            }            
        }
};

#endif

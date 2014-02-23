/**************************************************
 * FILENAME:        FileTree.cpp
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

#include "../inc/FileTree.h"

void FileTree::reset() {
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

FileTreeElement* FileTree::getNextElement(bool reset) {
    static FileTreeElement *pTmp = pRoot;
    FileTreeElement *pTmp2 = pTmp;
    if(pTmp == NULL) { 
        if(reset)
            pTmp = pRoot;
        return NULL;
    }
    pTmp = pTmp->pNext;

    if(reset)
        pTmp = pRoot;
    return pTmp2;            
}

FileTreeElement* FileTree::FindByName(char* name) {
    FileTreeElement *pTmp = pRoot; 
    
    while(pTmp != NULL) {
        if(strcmp(name, pTmp->Name) == 0)
            return pTmp;
        pTmp = pTmp->pNext;
    }
    return pTmp;
}

bool FileTree::removeElement(FileTreeElement* ptr) {
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

bool FileTree::addEntry(const char* name, struct stat attr) {
    FileTreeElement **pTmp;
    pTmp = &pRoot;
    if(*pTmp != NULL) {
        while((*pTmp)->pNext != NULL)
            pTmp = &((*pTmp)->pNext);

        (*pTmp)->pNext = (FileTreeElement*)malloc(sizeof(FileTreeElement));
        if((*pTmp)->pNext == NULL)
            return false;

        (*pTmp)->pNext->pNext = NULL;
        (*pTmp)->pNext->pPrev = (*pTmp);
        (*pTmp)->pNext->Name = strdup(name);
        if((*pTmp)->pNext->Name == NULL)
            return false;
        (*pTmp)->pNext->attr = attr;
    } else {    
        (*pTmp) = (FileTreeElement*)malloc(sizeof(FileTreeElement));
        if((*pTmp) == NULL)
            return false;

        (*pTmp)->pNext = NULL;
        (*pTmp)->pPrev = NULL;
        (*pTmp)->Name = strdup(name);
        if((*pTmp)->Name == NULL)
            return false;
        (*pTmp)->attr = attr;
    }
    return true;
}

const char* FileTree::serialize(FileTreeElement *pPtr) {
    char* ret = (char*)malloc(strlen(pPtr->Name) + sizeof(size_t) + sizeof(struct stat)+10);
    char* pTmp = ret;
    memcpy(ret, &(pPtr->attr), sizeof(struct stat));
    ret += sizeof(struct stat);
    *((size_t*)ret) = strlen(pPtr->Name);
    ret += sizeof(size_t);
    strcpy(ret, pPtr->Name);

    return pTmp;
}

const char* FileTree::getNextSerializedElement() {
    static FileTreeElement *pTmp = pRoot;
    if(pTmp == NULL)
        return NULL;
    const char* s = serialize(pTmp);
    pTmp = pTmp->pNext;

    return s;
}

bool FileTree::compare(FileTree *o) {
    FileTreeElement* ptr_o;
    bool ret = true;

    while((ptr_o = o->getNextElement()) != NULL) {
        bool differs = false;
        FileTreeElement *ptr_l = FindByName(ptr_o->Name);

        if(ptr_l == NULL) {
            ret = false;
            differs = true;
        } else {
            if(ptr_l->attr.st_uid != ptr_o->attr.st_uid) {
                ret = false;
                differs = true;
            } else if(ptr_l->attr.st_gid != ptr_o->attr.st_gid) {
                ret = false;
                differs = true;
            } else if(ptr_l->attr.st_mode != ptr_o->attr.st_mode) {
                ret = false;
                differs = true;
            } else if(ptr_l->attr.st_size != ptr_o->attr.st_size) {
                ret = false;
                differs = true;
            } else if(ptr_l->attr.st_mtime != ptr_o->attr.st_mtime) {
                /*  if a file is created, the mod-time of the parent directory is also changed. 
                    To prevent a backup of the whole directory, ignore modtime-changes of a directory */
                if((ptr_l->attr.st_mode & S_IFMT) != S_IFDIR) {
                    ret = false;
                    differs = true;
                }
            }
        }

        if(!differs) {
            removeElement(ptr_l);
            o->removeElement(ptr_o);
        }
    }    
    return ret;
}

void FileTree::printChanges(FileTree *o) {
    FileTreeElement* ptr_o;
    o->getNextElement(true);
    while((ptr_o = o->getNextElement()) != NULL) {
        FileTreeElement *ptr_l = FindByName(ptr_o->Name);

        if(ptr_l == NULL) {
            printf("A <%s>\n", ptr_o->Name);
        } else {
            if(ptr_l->attr.st_uid != ptr_o->attr.st_uid) {
                printf("M <%s>\n", ptr_o->Name);
            } else if(ptr_l->attr.st_gid != ptr_o->attr.st_gid) {
                printf("M <%s>\n", ptr_o->Name);
            } else if(ptr_l->attr.st_mode != ptr_o->attr.st_mode) {
                printf("M <%s>\n", ptr_o->Name);
            } else if(ptr_l->attr.st_size != ptr_o->attr.st_size) {
                printf("M <%s>\n", ptr_o->Name);
            } else if(ptr_l->attr.st_mtime != ptr_o->attr.st_mtime) {
                printf("M <%s>\n", ptr_o->Name);
            }
        }
    }

    FileTreeElement *ptr_l = getNextElement(true);
    while((ptr_l = getNextElement()) != NULL) {
        FileTreeElement *ptr_o = o->FindByName(ptr_l->Name);

        if(ptr_o == NULL) {
            printf("R <%s>\n", ptr_l->Name);
        }
    }
}

void FileTree::print() {
    FileTreeElement *ptr = pRoot;
    while(ptr != NULL) {
        printf("%s\n", ptr->Name);
        ptr = ptr->pNext;
    }            
}

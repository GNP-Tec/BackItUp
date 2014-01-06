/**************************************************
 * FILENAME:        fiterator.cpp
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

#include "../inc/fiterator.h"
#include <stdio.h>
#include <fcntl.h>  
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <utime.h>

#include <errno.h>

#define ERRWR_(x...)   {fprintf(stderr, x); return;}
#define ERRWR(x...)    { fprintf(stderr, x); return false; }

#ifdef SIZE
#warning DOES NOT WORK RIGHT!
#endif

FIterator::FIterator(Config *c, const char* directory, bool isRootDirectory) {
    struct stat attr;
    
    #ifdef SIZE
    size = 0;
    #endif

    #ifdef DEBUG
    printf("DEBUG: Backing up <%s>\n\r", directory);
    #endif

    if(isRootDirectory) {
        char* tmp = (char*)malloc(sizeof(char) * (strlen(directory) + 1));
        if(tmp == NULL) {
            c->log.Log(LogError, "Error allocating memory!\n");
            return ;
        }
        strcpy(tmp, directory);

        int maxlen = strlen(tmp);
        int cur = 1;

        while(cur < maxlen) {
            cur = strchr(tmp+cur, '/') - tmp;
            if(cur >= 0)
                tmp[cur] = 0;

            if(lstat(tmp, &attr)) {
                c->log.Log(LogError, "Error getting file stats for <%s>\n\r", tmp);
                return ;
            }

            c->FH->copyDirectory(tmp, tmp, &attr);

            if(cur < 0)
                break;
            tmp[cur] = '/';
            cur++;
        }
    }

    if(lstat(directory, &attr)) {
        c->log.Log(LogError, "Error getting file stats for <%s>\n\r", directory);
        return ;
    }

    if((attr.st_mode & S_IFMT) == S_IFDIR) {
        DIR *d = opendir(directory);
        if(d == NULL) {
             c->log.Log(LogError, "Error opening directory <%s>\n\r", directory);
            return ;
        }

        struct dirent* e;
        while((e = readdir(d)) != NULL) {
            if(e->d_name[0] == '.' && e->d_name[1] == '\0')
                continue;
            else if(e->d_name[0] == '.' && e->d_name[1] == '.' && e->d_name[2] == '\0')
                continue;

            char* src = (char*)malloc(sizeof(char) *( strlen(directory) + strlen(e->d_name) + 2));
            if(src == NULL) {
                c->log.Log(LogError, "Error allocating memory!\n");
                return ;
            }
            
            strcpy(src, directory);
            if(directory[strlen(directory)-1] != '/')
                strcat(src, "/");
            strcat(src, e->d_name);


            char* dest = (char*)malloc(sizeof(char) *(strlen(e->d_name) + strlen(directory) + 2));
            if(dest == NULL) {
                free(src);
                c->log.Log(LogError, "Error allocating memory!\n");
                return ;
            }
            
            strcpy(dest, directory);
            if(directory[strlen(directory)-1] != '/')
                strcat(dest, "/");
            strcat(dest, e->d_name);

            struct stat buf;
            if(lstat(src, &buf) < 0) {
                free(src);
                free(dest);
                c->log.Log(LogError, "Error getting file information <%s>!\n\r", src);
                return ;
            }
    
            #if SIZE
            size += attr.st_size;
            #endif

            switch(e->d_type) {
                case DT_DIR: {
                    c->FH->copyDirectory(src, dest, &buf);
                    FIterator f(c, src, false);
                    #ifdef SIZE
                    size += f.getSize();
                    #endif
                    break;
                }
                case DT_REG:
                case DT_LNK:
                case DT_CHR:
                case DT_BLK:
                    c->FH->copyFile(src, dest, &buf);
                    break;                
                default:
                    c->log.Log(LogWarning, "Filetype not implemented <%s>\n\r", src);
            }
            free(src);
            free(dest);
        } 

        closedir(d);
        #ifdef SIZE
        printf("Total size: %llu\n\r", size);
        #endif
    }
    
}



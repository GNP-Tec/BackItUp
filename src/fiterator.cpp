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

#warning links, ....
static bool backup(Config *c, const char* src, const char* dest) {
    int sfd, dfd;
    struct stat attr;

    #ifdef DEBUG
    printf("DEBUG: Copying <%s#%s>\n\r", src, dest);
    #endif

    if((sfd = open(src, O_RDONLY)) < 0) {
         c->log.Log(LogError, "Error opening file <%s>\n\r", src);
         return false;
    }
    
    if(lstat(src, &attr)) {
        close(sfd);
        c->log.Log(LogError, "Error getting file attributes for <%s>\n\r", src);
        return false;
    }

    if((attr.st_mode & S_IFMT) != S_IFREG) {
        close(sfd);
        ERRWR("Only regular files are supported!\n\r");
    }

    if((dfd = creat(dest, attr.st_mode)) < 0) {
        close(sfd);
        c->log.Log(LogError, "Error creating file <%s>\n\r", dest);
        return false;
    }

#warning Just works for linux
    if((sendfile(dfd, sfd, NULL, attr.st_size)) != attr.st_size) {
        close(dfd);
        close(sfd);
        c->log.Log(LogError, "Error writing file <%s>!\n\r", dest);
        return false;
    }
   
    close(dfd);
    close(sfd);

    struct utimbuf ut;
    ut.modtime = attr.st_mtime;
    ut.actime = 0;

    if(utime(dest, &ut) != 0)
         c->log.Log(LogWarning, "Couldn't set correct modification date for <%s>!\n\r", dest);

    return true;
}

#warning links, ...
// just create a new directory with the same rigths
static bool copyDir(Config *c, const char* src, const char* dest) {
    struct stat attr;

    #ifdef DEBUG
    printf("DEBUG: Creating directory <%s#%s>\n\r", src, dest);
    #endif

    if(lstat(src, &attr)) {
        c->log.Log(LogError, "Error getting file attributes <%s>\n\r", src);
        return false;
    }

    if((attr.st_mode & S_IFMT) != S_IFDIR) {
        c->log.Log(LogError, "Not a directory <%s>!\n\r", src);
        return false;
    }

    if(mkdir(dest, attr.st_mode) != 0) {
        c->log.Log(LogError, "Couldn't create directory <%s>\n\r", dest);
        return false;
    }

    struct utimbuf ut;
    ut.modtime = attr.st_mtime;
    ut.actime = 0;

    if(utime(dest, &ut) != 0)
         c->log.Log(LogWarning, "Couldn't set correct modification date for <%s>!\n\r", dest);

    return true;
}

FIterator::FIterator(Config *c, const char* directory, bool isRootDirectory) {
    struct stat attr;
    
    #ifdef SIZE
    size = 0;
    #endif

    #ifdef DEBUG
    printf("DEBUG: Backing up <%s>\n\r", directory);
    #endif

    if(isRootDirectory) {
#warning CHECK POINTER MEM
        char* tmp = (char*)malloc(sizeof(char) * (strlen(c->getBackupDestination()) + strlen(directory) + 2));
        strcpy(tmp, c->getBackupDestination());
        strcat(tmp, "/");
        strcat(tmp, directory);

        int maxlen = strlen(tmp);
        int cur = strlen(c->getBackupDestination()) + (directory[0] == '/' ? 2 : 1);

        while(cur < maxlen) {
            cur = strchr(tmp+cur, '/') - tmp;
            if(cur >= 0)
                tmp[cur] = 0;
            copyDir(c, tmp + strlen(c->getBackupDestination()), tmp);

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
            strcat(src, "/");
            strcat(src, e->d_name);


            char* dest = (char*)malloc(sizeof(char) *( strlen(c->getBackupDestination()) + strlen(e->d_name) + strlen(directory) + 3));
            if(dest == NULL) {
                free(src);
                c->log.Log(LogError, "Error allocating memory!\n");
                return ;
            }
            
            strcpy(dest, c->getBackupDestination());
            strcat(dest, "/");
            strcat(dest, directory);
            strcat(dest, "/");
            strcat(dest, e->d_name);

#warning OPTIMIZE
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
// END OPTIMIZE

//            printf("%s (%i) - %li\n", src, e->d_type, buf.st_size);
            switch(e->d_type) {
                case DT_DIR: {
                    copyDir(c, src, dest);
                    FIterator f(c, src, false);
                    #ifdef SIZE
                    size += f.getSize();
                    #endif
                    break;
                }
                case DT_REG:
                case DT_LNK:
                    backup(c, src, dest);
 //                   printf("Reg\n\r");  
                    break;                
                default:
                    #warning IMPLEMENT
                    ERRWR_("Not implemented yet\n\r");
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



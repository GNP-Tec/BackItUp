/**************************************************
 * FILENAME:        rfilehandler.cpp
 * PROJECT:         BackItUp
 *
 * AUTHOR:          Philipp Doblhofer
 * WEB:             www.gnp-tec.net
 * START DATE:      2014-Jan-17
 *
 **************************************************
 * DESCRIPTION:
 * Regular file handler
 *************************************************/

#include "../inc/filehandler.h"
#include "../inc/rfilehandler.h"

#include <stdio.h>
#include <fcntl.h>  
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <utime.h>

#include <errno.h>

bool RFileHandler::Init(Config* conf) {
    if(conf == NULL)
        return false;
    c = conf;

    if(c->getType() == TYPE_UNCOMPRESSED) {
        return true;
    }
    return false;    
}

void RFileHandler::Finalize() {
}

bool RFileHandler::copyDirectory(const char* src, const char* dest, struct stat *attr) {
    if((attr->st_mode & S_IFMT) != S_IFDIR) {
        c->log.Log(LogError, "Not a directory <%s>!\n\r", src);
        return false;
    }

    char* temp = (char*)malloc(sizeof(char) *( strlen(c->getBackupDestination()) + strlen(dest) + 2));
    if(dest == NULL) {
        c->log.Log(LogError, "Error allocating memory!\n");
        return false;
    }
    
    strcpy(temp, c->getBackupDestination());
    if(c->getBackupDestination()[strlen(c->getBackupDestination())-1] != '/')
        strcat(temp, "/");
    strcat(temp, dest);

    #ifdef DEBUG
    printf("DEBUG: Creating directory <%s#%s>\n\r", src, temp);
    #endif

    if(mkdir(temp, attr->st_mode) != 0) {
        c->log.Log(LogError, "Couldn't create directory <%s>\n\r", temp);
        return false;
    }

    if(chown(temp, attr->st_uid, attr->st_gid) != 0) {
        c->log.Log(LogWarning, "Couldn't set correct UID & GID of <%s>\n\r", temp);
    }

    struct utimbuf ut;
    ut.modtime = attr->st_mtime;
    ut.actime = 0;

    if(utime(temp, &ut) != 0)
         c->log.Log(LogWarning, "Couldn't set correct modification date for <%s>!\n\r", temp);

    return true;
}

bool RFileHandler::copyFile(const char* src, const char* dest, struct stat* attr) {
    int sfd;
    char buf[8192];
        
    int dfd;

    char* temp = (char*)malloc(sizeof(char) *( strlen(c->getBackupDestination()) + strlen(dest) + 2));
    if(dest == NULL) {
        c->log.Log(LogError, "Error allocating memory!\n");
        return false;
    }
    
    strcpy(temp, c->getBackupDestination());
    if(c->getBackupDestination()[strlen(c->getBackupDestination())-1] != '/')
        strcat(temp, "/");
    strcat(temp, dest);

    #ifdef DEBUG
    printf("DEBUG: Copying <%s#%s>\n\r", src, temp);
    #endif

    if((attr->st_mode & S_IFMT) == S_IFREG) {
        if((sfd = open(src, O_RDONLY)) < 0) {
             c->log.Log(LogError, "Error opening file <%s>\n\r", src);
             return false;
        }

        if((dfd = creat(temp, attr->st_mode)) < 0) {
            close(sfd);
            c->log.Log(LogError, "Error creating file <%s>\n\r", temp);
            return false;
        }

        /*#warning Just works for linux*/
        if((sendfile(dfd, sfd, NULL, attr->st_size)) != attr->st_size) {
            close(dfd);
            close(sfd);
            c->log.Log(LogError, "Error writing file <%s>!\n\r", temp);
            return false;
        }
       
        if(fchown(dfd, attr->st_uid, attr->st_gid) != 0) {
            c->log.Log(LogWarning, "Couldn't set correct UID & GID of <%s>\n\r", temp);
        }

        close(dfd);
        close(sfd);
        struct utimbuf ut;
        ut.modtime = attr->st_mtime;
        ut.actime = 0;

        if(utime(temp, &ut) != 0)
             c->log.Log(LogWarning, "Couldn't set correct modification date for <%s>!\n\r", temp);

    } else if((attr->st_mode & S_IFMT) == S_IFLNK) {
        buf[readlink(src, buf, sizeof(buf))] = 0;
        if(symlink(buf, temp) != 0) {
            c->log.Log(LogError, "Error creating symlink <%s>\n\r", temp);
            return false;
        }

        if(lchown(temp, attr->st_uid, attr->st_gid) != 0) {
            c->log.Log(LogWarning, "Couldn't set correct UID & GID of <%s>\n\r", temp);
        }
    } else if((attr->st_mode & S_IFMT) == S_IFCHR || (attr->st_mode & S_IFMT) == S_IFBLK) {
        if(mknod(temp, attr->st_mode, attr->st_rdev) != 0) {
            c->log.Log(LogError, "Error creating device file <%s>\n\r", temp);
            perror("X:\n\r");
            return false;
        }

        if(chown(temp, attr->st_uid, attr->st_gid) != 0) {
            c->log.Log(LogWarning, "Couldn't set correct UID & GID of <%s>\n\r", temp);
        }
    } else {
        c->log.Log(LogError, "Only regular, link and device files are supported <%s>!\n\r", src);
        return false;
    }

    return true;
}

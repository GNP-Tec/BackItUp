/**************************************************
 * FILENAME:        filehandler.cpp
 * PROJECT:         BackItUp
 *
 * AUTHOR:          Philipp Doblhofer
 * WEB:             www.gnp-tec.net
 * START DATE:      2014-Jan-4
 *
 **************************************************
 * DESCRIPTION:
 * Simplifies the access to regular files and 
 * archives
 *************************************************/

#include "../inc/filehandler.h"

#include <stdio.h>
#include <fcntl.h>  
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <utime.h>
#include <archive_entry.h>

#include <errno.h>

#warning REMOVE > LOG
#define ERRWR(x...)    { fprintf(stderr, x); return false; }

bool FileHandler::Init(Config* conf) {
    if(conf == NULL)
        return false;
    c = conf;
    if(c->getType() == TYPE_UNCOMPRESSED) {
        return true;
    } else if(c->getType() == TYPE_COMPRESSED) {
        a = archive_write_new();
        archive_write_add_filter_gzip(a);
        archive_write_set_format_pax_restricted(a);
        archive_write_open_filename(a, c->getBackupDestination());
        return true;
    }

    return false;    
}

void FileHandler::Finalize() {
    if(c != NULL && c->getType() == TYPE_COMPRESSED) {
        archive_write_close(a);
        archive_write_free(a);
    }
}

#warning links, ...
bool FileHandler::copyDirectory(const char* src, const char* dest) {
    struct stat attr;

    if(lstat(src, &attr)) {
        c->log.Log(LogError, "Error getting file attributes <%s>\n\r", src);
        return false;
    }

    if((attr.st_mode & S_IFMT) != S_IFDIR) {
        c->log.Log(LogError, "Not a directory <%s>!\n\r", src);
        return false;
    }

    if(c->getType() == TYPE_UNCOMPRESSED) {
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

        if(mkdir(temp, attr.st_mode) != 0) {
            c->log.Log(LogError, "Couldn't create directory <%s>\n\r", temp);
            return false;
        }

        struct utimbuf ut;
        ut.modtime = attr.st_mtime;
        ut.actime = 0;

        if(utime(temp, &ut) != 0)
             c->log.Log(LogWarning, "Couldn't set correct modification date for <%s>!\n\r", temp);

        return true;
    } else if(c->getType() == TYPE_COMPRESSED) {
        struct archive_entry *entry;

        entry = archive_entry_new();
        archive_entry_set_pathname(entry, dest);
        archive_entry_set_size(entry, attr.st_size);
        archive_entry_set_filetype(entry, AE_IFDIR);
        archive_entry_set_perm(entry, attr.st_mode & 0777);
        archive_entry_set_mtime(entry, attr.st_mtime, 0);
        archive_write_header(a, entry);
        archive_entry_free(entry);

        return true;
    }
    return false;
}

#warning links, ....
bool FileHandler::copyFile(const char* src, const char* dest) {
    int sfd;
    struct stat attr;

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

    if(c->getType() == TYPE_UNCOMPRESSED) {
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

        if((dfd = creat(temp, attr.st_mode)) < 0) {
            close(sfd);
            c->log.Log(LogError, "Error creating file <%s>\n\r", temp);
            return false;
        }

        /*#warning Just works for linux*/
        if((sendfile(dfd, sfd, NULL, attr.st_size)) != attr.st_size) {
            close(dfd);
            close(sfd);
            c->log.Log(LogError, "Error writing file <%s>!\n\r", temp);
            return false;
        }
       
        close(dfd);
        close(sfd);

        struct utimbuf ut;
        ut.modtime = attr.st_mtime;
        ut.actime = 0;

        if(utime(temp, &ut) != 0)
             c->log.Log(LogWarning, "Couldn't set correct modification date for <%s>!\n\r", temp);

        return true;
    } else if(c->getType() == TYPE_COMPRESSED) {
        struct archive_entry *entry;

        entry = archive_entry_new();
        archive_entry_set_pathname(entry, dest);
        archive_entry_set_size(entry, attr.st_size);
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, attr.st_mode && 0777);
        archive_entry_set_mtime(entry, attr.st_mtime, 0);
        archive_write_header(a, entry);

        char buf[8192];
        int len;

        len = read(sfd, buf, sizeof(buf));
        while (len > 0) {
            archive_write_data(a, buf, len);
            len = read(sfd, buf, sizeof(buf));
        }
        close(sfd);

        archive_entry_free(entry);

        return true;
    }
    return false;
}

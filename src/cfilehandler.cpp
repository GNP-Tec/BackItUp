/**************************************************
 * FILENAME:        cfilehandler.cpp
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

#include "../inc/filehandler.h"
#include "../inc/cfilehandler.h"

#include <stdio.h>
#include <fcntl.h>  
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <utime.h>
#include <archive_entry.h>

#include <errno.h>

bool CFileHandler::Init(Config* conf) {
    if(conf == NULL)
        return false;
    c = conf;

    if(c->getType() == TYPE_COMPRESSED) {
        a = archive_write_new();
        if(archive_write_add_filter_gzip(a) != ARCHIVE_OK || archive_write_set_format_pax_restricted(a) != ARCHIVE_OK || \
                archive_write_open_filename(a, c->getBackupDestination()) != ARCHIVE_OK) {
            c->log.Log(LogError, "Error creating archive file <%s>\n\r", c->getBackupDestination());
            return false;
        }
        return true;
    }

    return false;    
}

void CFileHandler::Finalize() {
    if(c != NULL && c->getType() == TYPE_COMPRESSED) {
        archive_write_close(a);
        archive_write_free(a);
    }
}

bool CFileHandler::copyDirectory(const char* src, const char* dest, struct stat *attr) {
    if((attr->st_mode & S_IFMT) != S_IFDIR) {
        c->log.Log(LogError, "Not a directory <%s>!\n\r", src);
        return false;
    }
    
    struct archive_entry *entry;

    entry = archive_entry_new();
    if(entry == NULL) {
        c->log.Log(LogError, "Error creating file in archive <%s>\n\r", dest);
        return false;
    }        
    archive_entry_set_pathname(entry, dest);
    archive_entry_set_size(entry, attr->st_size);
    archive_entry_set_filetype(entry, AE_IFDIR);
    archive_entry_set_perm(entry, attr->st_mode & 0777);
    archive_entry_set_mtime(entry, attr->st_mtime, 0);
    archive_entry_set_uid(entry, attr->st_uid);
    archive_entry_set_gid(entry, attr->st_gid);
    if(archive_write_header(a, entry) != ARCHIVE_OK) {
        archive_entry_free(entry);
        c->log.Log(LogError, "Error creating directory in archive <%s>\n\r", dest);
        return false;
    }
    archive_entry_free(entry);

    return true;
}

bool CFileHandler::copyFile(const char* src, const char* dest, struct stat* attr) {
    int sfd;
    char buf[8192];   
    int len;
    struct archive_entry *entry;

    entry = archive_entry_new();
    if(entry == NULL) {
        c->log.Log(LogError, "Error creating file in archive <%s>\n\r", dest);
        return false;
    }
    archive_entry_set_pathname(entry, dest);
    archive_entry_set_size(entry, attr->st_size);
    archive_entry_set_perm(entry, attr->st_mode);
    archive_entry_set_mtime(entry, attr->st_mtime, 0);
    archive_entry_set_uid(entry, attr->st_uid);
    archive_entry_set_gid(entry, attr->st_gid);
    if((attr->st_mode & S_IFMT) == S_IFREG) {
        archive_entry_set_filetype(entry, AE_IFREG);
    } else if((attr->st_mode & S_IFMT) == S_IFLNK) {
        archive_entry_set_filetype(entry, AE_IFLNK);
        buf[readlink(src, buf, sizeof(buf))] = 0;
        archive_entry_set_symlink(entry, buf);
    }else {
        c->log.Log(LogError, "Only regular and link files are supported <%s>!\n\r", src);
        return false;
    }
    if(archive_write_header(a, entry) != ARCHIVE_OK) {
        c->log.Log(LogError, "Error creating file in archive <%s>\n\r", dest);
        return false;
    }

    if((attr->st_mode & S_IFMT) == S_IFREG) {   
        if((sfd = open(src, O_RDONLY)) < 0) {
             c->log.Log(LogError, "Error opening file <%s>\n\r", src);
             return false;
        }
        len = read(sfd, buf, sizeof(buf));
        while (len > 0) {
            if(archive_write_data(a, buf, len) != len) {
                c->log.Log(LogError, "Error copying file into archive <%s>\n\r", dest);
            }
            len = read(sfd, buf, sizeof(buf));
        }
        close(sfd);
    }

    archive_entry_free(entry);

    return true;
}


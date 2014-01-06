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
        if(archive_write_add_filter_gzip(a) != ARCHIVE_OK || archive_write_set_format_pax_restricted(a) != ARCHIVE_OK || \
                archive_write_open_filename(a, c->getBackupDestination()) != ARCHIVE_OK) {
            c->log.Log(LogError, "Error creating archive file <%s>\n\r", c->getBackupDestination());
            return false;
        }
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

bool FileHandler::copyDirectory(const char* src, const char* dest, struct stat *attr) {
    if((attr->st_mode & S_IFMT) != S_IFDIR) {
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
    } else if(c->getType() == TYPE_COMPRESSED) {
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
    return false;
}

bool FileHandler::copyFile(const char* src, const char* dest, struct stat* attr) {
    int sfd;
    char buf[8192];
        
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
    } else if(c->getType() == TYPE_COMPRESSED) {
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
    return false;
}

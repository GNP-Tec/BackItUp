/**************************************************
 * FILENAME:        compressed.cpp
 * PROJECT:         BackItUp
 *
 * AUTHOR:          Philipp Doblhofer
 * WEB:             www.gnp-tec.net
 * START DATE:      2014-Feb-01
 *
 **************************************************
 * DESCRIPTION:
 * The compressed backup handler (.tar.gz)
 *************************************************/

#include "../inc/compressed.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <utime.h>
#include <dirent.h>

#warning ADD "files" TO ARCHIVE!

bool CompressedBackup::OpenBackup(const char* path) {
    root_file = (char*)malloc(strlen(path) + 30);
    if(root_file == NULL) {
        Log.Log(LogError, "Error allocating memory!\n");
        return false;
    }

    strcpy(root_file, path);   
    return true;
}

bool CompressedBackup::PrintConfig() {
    struct archive *a;
    struct archive_entry *entry;

    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
    if(archive_read_open_filename(a, root_file, 10240) != ARCHIVE_OK) {
        Log.Log(LogError, "Error opening archive <%s>\n", root_file);
        return false;
    }
    while(archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        if(strcmp("config.xml", archive_entry_pathname(entry)) == 0) {

            char *buf = (char*)malloc(archive_entry_size(entry));
            archive_read_data(a, buf, archive_entry_size(entry));

            puts(buf);
            break;
        }
        archive_read_data_skip(a);
    }
    if (archive_read_free(a) != ARCHIVE_OK)
        return false;

    return true;
}

bool CompressedBackup::CloseBackup() {    
    free(root_file);
    return true;
}


bool CompressedBackup::Initialize() {
    // create backup directory
    root_file = (char*)malloc(strlen(b->c.GetDestination())+20+10); // + <date> + "data"/"config.xml"/...
    if(root_file == NULL) {
        Log.Log(LogError, "Error allocating memory!\n");
        return false;
    }
    strcpy(root_file, b->c.GetDestination());

    // Create backup directory if it doesn't exist
    if(mkdir(root_file, 0755) != 0 && errno != EEXIST) {
        Log.Log(LogError, "Couldn't create directory <%s>\n\r", root_file);
        return false;
    }

    if(root_file[strlen(root_file)-1] != '/')
        strcat(root_file, "/");

    time_t t = time(0);
    struct tm *tmp = localtime(&t);
    strftime(root_file + strlen(root_file), strlen(b->c.GetDestination())+20, "%Y%m%d-%H%M%S.tar.gz", tmp);

    a = archive_write_new();
    if(archive_write_add_filter_gzip(a) != ARCHIVE_OK || archive_write_set_format_pax_restricted(a) != ARCHIVE_OK || \
            archive_write_open_filename(a, root_file) != ARCHIVE_OK) {
        Log.Log(LogError, "Error creating archive file <%s>\n\r", root_file);
        return false;
    }
    Log.Log(LogInfo, "Backup-Location: %s\n", root_file);

    struct archive_entry *entry;

    entry = archive_entry_new();
    if(entry == NULL) {
        Log.Log(LogError, "Error creating folder in archive <%s>\n\r", "data");
        return false;
    }
    archive_entry_set_pathname(entry, "data");
    archive_entry_set_perm(entry, 0777);
    archive_entry_set_filetype(entry, AE_IFDIR);
    if(archive_write_header(a, entry) != ARCHIVE_OK) {
        archive_entry_free(entry);
        Log.Log(LogError, "Error creating directory in archive <%s>\n\r", "data");
        perror("A:\n");
        return false;
    }
    archive_entry_free(entry);

    strcpy(root_file, "config.xml");
    copyFile(b->GetConfigFile(), root_file);

    return true;
}

bool CompressedBackup::copyFile(const char* src, const char* dest) {
    char buf[8192];
    int len;
    int sfd;
    struct stat attr;

    Log.Log(LogInfo, "Copying <%s> to <%s>\n", src, dest);

    if(lstat(src, &attr) < 0) {
        Log.Log(LogError, "Error getting file information <%s>!\n\r", src);
        return false;
    }

    struct archive_entry *entry;

    entry = archive_entry_new();
    if(entry == NULL) {
        Log.Log(LogError, "Error creating file in archive <%s>\n\r", dest);
        return false;
    }
    archive_entry_set_pathname(entry, dest);    
    archive_entry_set_size(entry, attr.st_size);
    archive_entry_set_perm(entry, attr.st_mode);
    archive_entry_set_mtime(entry, attr.st_mtime, 0);
    archive_entry_set_uid(entry, attr.st_uid);
    archive_entry_set_gid(entry, attr.st_gid);

    if((attr.st_mode & S_IFMT) == S_IFREG) {
        archive_entry_set_filetype(entry, AE_IFREG);
    } else if((attr.st_mode & S_IFMT) == S_IFLNK) {
        archive_entry_set_filetype(entry, AE_IFLNK);
        buf[readlink(src, buf, sizeof(buf))] = 0;
        archive_entry_set_symlink(entry, buf);
    } else if((attr.st_mode & S_IFMT) == S_IFDIR) {
        archive_entry_set_filetype(entry, AE_IFDIR);
    } else {
        Log.Log(LogError, "Only folders, regular and link are supported <%s>!\n", src);
        return false;
    }
    if(archive_write_header(a, entry) != ARCHIVE_OK) {
        Log.Log(LogError, "Error creating file in archive <%s>\n\r", dest);
        return false;
    }

    if((attr.st_mode & S_IFMT) == S_IFREG) {
        if((sfd = open(src, O_RDONLY)) < 0) {
             Log.Log(LogError, "Error opening file <%s>\n\r", src);
             return false;
        }
        len = read(sfd, buf, sizeof(buf));
        while (len > 0) {
            if(archive_write_data(a, buf, len) != len) {
                Log.Log(LogError, "Error copying file into archive <%s>\n\r", dest);
            }
            len = read(sfd, buf, sizeof(buf));
        }
        close(sfd);
    }

    archive_entry_free(entry);

    return true;     
}

bool CompressedBackup::Finalize() {
    archive_write_close(a);
    archive_write_free(a);
    free(root_file);
    return true;
}

bool CompressedBackup::addFolder(const char* path, bool init) {
    if(path == NULL || strlen(path) == 0)
        return false;

    struct stat attr;
    if(lstat(path, &attr)<0) {
        Log.Log(LogError, "Error getting file information <%s>!\n\r", path);
        return false;        
    }

    if((attr.st_mode & S_IFMT) == S_IFDIR) {
        DIR *d = opendir(path);
        if(d == NULL) {
            Log.Log(LogError, "Error opening directory <%s>\n", path);
            closedir(d);
            return false;
        }

        struct dirent *e;
        while((e = readdir(d)) != NULL) {
            if(e->d_name[0] == '.' && e->d_name[1] == '\0')
                continue;
            else if(e->d_name[0] == '.' && e->d_name[1] == '.' && e->d_name[2] == '\0')
                continue;

            char* src = (char*)malloc(sizeof(char)*(strlen(path) + strlen(e->d_name)+2));
            if(src == NULL) {
                Log.Log(LogError, "Error allocating memory!\n");
                closedir(d);
                return false;
            }
            strcpy(src, path);
            if(src[strlen(src)-1] != '/')
                strcat(src, "/");
            strcat(src, e->d_name);

#warning UTF8 (Umlaute) in Compressed and regular
            char* dest = (char*)malloc(sizeof(char)*(strlen(path)+strlen(e->d_name)+10));
            if(dest == NULL) {
                Log.Log(LogError, "Error allocating memory!\n");
                free(src);
                closedir(d);
                return false;
            }
            strcpy(dest, "data/");
            char* ptr = dest + strlen(dest)+1;
            if(path[0] == '/')
                dest[strlen(dest)-1] = 0;
            strcat(dest, path);
            if(dest[strlen(dest)-1] != '/')
                strcat(dest, "/");
            strcat(dest, e->d_name);

            // create parent directories
            while(init && *ptr != 0 && strchr(ptr, '/') != NULL) {
                ptr += strchr(ptr, '/') - ptr;
                *ptr = 0;
                copyFile(dest + 4, dest);
                *ptr++ = '/';
            }
            init = false;

            copyFile(src, dest);
            addFolder(src, false);

            free(src);
            free(dest);
        }
        closedir(d);
    } else if(init) {
        Log.Log(LogError, "Only Folders are allowed! (%s)\n", path);
        return false;
    }
    return true;
}

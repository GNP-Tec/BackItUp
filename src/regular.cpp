/**************************************************
 * FILENAME:        regular.cpp
 * PROJECT:         BackItUp
 *
 * AUTHOR:          Philipp Doblhofer
 * WEB:             www.gnp-tec.net
 * START DATE:      2014-Jan-25
 *
 **************************************************
 * DESCRIPTION:
 * The regular backup handler
 *************************************************/

#include "../inc/regular.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <utime.h>
#include <dirent.h>

bool RegularBackup::OpenBackup(const char* path) {
    root_dir = (char*)malloc(strlen(path) + 30);
    if(root_dir == NULL) {
        Log.Log(LogError, "Error allocating memory!\n");
        return false;
    }

    strcpy(root_dir, path);
   
    return true;
}

bool RegularBackup::PrintConfig() {
    char* ptr = root_dir + strlen(root_dir);
    strcat(root_dir, "/config.xml");
    
    FILE *fd = fopen(root_dir, "r");
    if(fd == NULL) {
        Log.Log(LogError, "Error opening config file <%s>\n", root_dir);
        return false;
    }
    
    char c;
    while((c = fgetc(fd)) && !feof(fd)) {
        putchar(c);
    }

    fclose(fd);

    *ptr = 0;
    return true;
}

FileTree RegularBackup::GetFileTree() {
    char* ptr = root_dir + strlen(root_dir);
    strcat(root_dir, "/files");
    
    int sfd;
    
    if((sfd = open(root_dir, O_RDONLY))<0) {
        Log.Log(LogError, "Error opening file <%s>\n", root_dir);
        return ft;
    }


    struct stat attr;
    unsigned int s;
    char *name; 

    while(1) {
        if(read(sfd, &attr, sizeof(struct stat))<=0) break;
        if(read(sfd, &s, sizeof(size_t))<=0) break;
        name = (char*)malloc(s + 10);
        if(name == NULL) {
            Log.Log(LogError, "Error allocating memory!\n");
            continue;
        }
        if(read(sfd, name, s)<=0) break;
        name[s] = 0;
        //printf("%s\n", name);
        ft.addEntry(name, attr);
        free(name);
    }

    close(sfd);
     
    *ptr = 0;
    return ft;
}

bool RegularBackup::CloseBackup() {    
    free(root_dir);
    return true;
}

bool RegularBackup::Initialize() {
    // create backup directory
    root_dir = (char*)malloc(strlen(b->c.GetDestination())+20+10); // + <date> + "data"/"config.xml"/...
    if(root_dir == NULL) {
        Log.Log(LogError, "Error allocating memory!\n");
        return false;
    }
    strcpy(root_dir, b->c.GetDestination());

    // Create backup directory if it doesn't exist
    if(mkdir(root_dir, 0755) != 0 && errno != EEXIST) {
        Log.Log(LogError, "Couldn't create directory <%s>\n\r", root_dir);
        return false;
    }

    if(root_dir[strlen(root_dir)-1] != '/')
        strcat(root_dir, "/");

    time_t t = time(0);
    struct tm *tmp = localtime(&t);
    strftime(root_dir + strlen(root_dir), strlen(b->c.GetDestination())+20, "%Y%m%d-%H%M%S/", tmp);
    
    if(mkdir(root_dir, 0755) != 0) {
        Log.Log(LogError, "Couldn't create directory <%s>\n\r", root_dir);
        return false;
    }

    Log.Log(LogInfo, "Backup-Location: %s\n", root_dir);
    
    char* ptr = root_dir + strlen(root_dir);
    strcat(root_dir, "data");
    if(mkdir(root_dir, 0755) != 0) {
        Log.Log(LogError, "Couldn't create directory <%s>\n\r", root_dir);
        return false;
    }
    *ptr = 0;

    // Copy config  
    strcat(root_dir, "config.xml");
    copyFile(b->GetConfigFile() ,root_dir);
    *ptr = 0;

    return true;
}

bool RegularBackup::copyFile(const char* src, const char* dest) {
    int sfd, dfd;
    struct stat attr;

    Log.Log(LogInfo, "Copying <%s> to <%s>\n", src, dest);

    if(lstat(src, &attr) < 0) {
        Log.Log(LogError, "Error getting file information <%s>!\n\r", src);
        return false;
    }

    ft.addEntry(src, attr);

    if((attr.st_mode & S_IFMT) == S_IFREG) {
        if((sfd = open(src, O_RDONLY))<0) {
            Log.Log(LogError, "Error opening file <%s>\n", src);
            return false;
        }
        
        if((dfd = creat(dest, attr.st_mode))<0) {
            close(sfd);
            Log.Log(LogError, "Error creating file <%s>\n", dest);
            return false;
        }

        if((sendfile(dfd, sfd, NULL, attr.st_size)) != attr.st_size) {
            close(dfd);
            close(sfd);
            Log.Log(LogError, "Error copying file <%s> to <%s>\n", src, dest);
            return false;
        }

        if(fchown(dfd, attr.st_uid, attr.st_gid) != 0)
            Log.Log(LogWarning, "Couldn't set correct UID & GID of <%s>\n", dest);

        close(dfd);
        close(sfd);

        struct utimbuf ut;
        ut.modtime = attr.st_mtime;
        ut.actime = 0;

        if(utime(dest, &ut) != 0)
            Log.Log(LogWarning, "Couldn't set correct modification time for <%s>\n", dest);
    } else if((attr.st_mode & S_IFMT) == S_IFLNK) {
        char buf[8192];
        buf[readlink(src, buf, sizeof(buf))] = 0;
        if(symlink(buf, dest) != 0) {
            Log.Log(LogError, "Error creating symlink <%s>\n", dest);
            return false;
        }

        if(lchown(dest, attr.st_uid, attr.st_gid) != 0)
            Log.Log(LogWarning, "Couldn't set correct UID & GID of <%s>\n", dest);
    } else if((attr.st_mode & S_IFMT) == S_IFCHR || (attr.st_mode & S_IFMT) == S_IFBLK) {
        if(mknod(dest, attr.st_mode, attr.st_rdev) != 0) {
            Log.Log(LogError, "Error creating device file <%s>\n", dest);
            return false;
        }

        if(chown(dest, attr.st_uid, attr.st_gid) != 0)
            Log.Log(LogWarning, "Couldn't set correct UID & GID of <%s>\n", dest);
    } else if((attr.st_mode & S_IFMT) == S_IFDIR) {
        if(mkdir(dest, attr.st_mode) != 0) {
            Log.Log(LogError, "Error creating directory <%s>\n", dest);
            return false;
        }

        struct utimbuf ut;
        ut.modtime = attr.st_mtime;
        ut.actime = 0;

        if(utime(dest, &ut) != 0)
            Log.Log(LogWarning, "Couldn't set correct modification time for <%s>\n", dest);

        if(chown(dest, attr.st_uid, attr.st_gid) != 0)
            Log.Log(LogWarning, "Couldn't set correct UID & GID of <%s>\n", dest);
    } else {
        Log.Log(LogError, "Only folders, regular, link and device files are supported <%s>!\n", src);
        return false;
    }

    return true;
        
}

bool RegularBackup::Finalize() {
    int dfd;

    char* ptr = root_dir + strlen(root_dir);
    strcat(ptr, "files");
    printf("PTR: %s\n", root_dir);

    if((dfd = creat(root_dir, 0777))<0) {
        Log.Log(LogError, "Error creating file <%s>\n", root_dir);
        return false;
    }

    char *s;
    while((s = (char*)ft.getNextSerializedElement()) != NULL) {
        unsigned int size = sizeof(struct stat) + (size_t)(*((size_t*)(s+sizeof(struct stat))) + sizeof(size_t));
        write(dfd, s, size);
    }

    close(dfd);   
    free(root_dir);
    return true;
}

bool RegularBackup::addFolder(const char* path, bool init) {
    if(path == NULL || strlen(path) == 0)
        return false;

    //Log.Log(LogInfo, "Backup <%s>\n", path);

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

            char* dest = (char*)malloc(sizeof(char)*(strlen(root_dir)+strlen(path)+strlen(e->d_name)+10));
            if(dest == NULL) {
                Log.Log(LogError, "Error allocating memory!\n");
                free(src);
                closedir(d);
                return false;
            }
            strcpy(dest, root_dir);
            strcat(dest, "data/");
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
                //Log.Log(LogInfo, "D: %s %s\n", dest + strlen(root_dir) + 4, dest); 
                copyFile(dest + strlen(root_dir) + 4, dest);
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

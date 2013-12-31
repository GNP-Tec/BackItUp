#include "fiterator.h"
#include <stdio.h>
#include <fcntl.h>  
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#define ERRWR_(x...)   {fprintf(stderr, x); return;}

#if 0
#warning links, ....
bool backup(const char* src, const char* dest) {
    int sfd, dfd;
    struct stat attr;

    if((sfd = open(src, O_RDONLY)) < 0) {
        fprintf(stderr, "Error opening file <%s>\n\r", src);
        return false;
    }
    
    if(lstat(src, &attr)) {
        close(sfd);
        fprintf(stderr, "Error getting file attributes\n\r");
        return false;
    }

    if((attr.st_mode & S_IFMT) != S_IFREG) {
        close(sfd);
        fprintf(stderr, "Only regular files are supported!\n\r");
        return false;
    }

    if((dfd = creat(dest, attr.st_mode)) < 0) {
        close(sfd);
        fprintf(stderr, "Error creating file <%s>\n\r", dest);
        return false;
    }

#warning Just works for linux
    if((sendfile(dfd, sfd, NULL, attr.st_size)) != attr.st_size) {
        fprintf(stderr, "Error writing file!\n\r");
        close(dfd);
        close(sfd);
        return false;
    }
   
    close(dfd);
    close(sfd);
    return true;
}
#endif

FIterator::FIterator(const char* directory, bool isRootDirectory) {
    struct stat attr;

    #ifdef DEBUG
    printf("DEBUG: Backing up <%s>\n\r", directory);
    #endif

    if(lstat(directory, &attr))
        ERRWR_("Error getting file stats\n\r");

    if((attr.st_mode & S_IFMT) == S_IFDIR) {
        printf("Directory\n\r");

        DIR *d = opendir(directory);
        if(d == NULL)
            ERRWR_("Error opening directory (%s)\n\r", directory);

        struct dirent* e;
        while(e = readdir(d)) {
            printf("~%s/%s\n\r", directory, e->d_name);

            char* tmp = malloc(sizeof(char) * strlen(directory) + strlen(e->d_name) + 2);
            if(tmp == NULL)
                ERRWR_("Error allocating memory!\n");
            
            strcpy(...

            FIterator f(String(directory) + String("/") + String(e->d_name));
            free(tmp);
        } 

        closedir(d);
    }

    
}



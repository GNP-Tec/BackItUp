#include <stdio.h>

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>

#include "config.h"
#include "fiterator.h"

#define ERR(x...)   {fprintf(stderr, x);}

#warning REMOVE SYMBOLIC NAMES

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

#if 0
void readConfig(const char* file) {
    xmlDocPtr doc;
    xmlNodePtr cur;
    
    doc = xmlParseFile(file);

    if(doc == NULL) {
        fprintf(stderr, "Error parsing config file!\n\r");
        return;
    }

    cur = xmlDocGetRootElement(doc);
    
    if(cur == NULL) {
        fprintf(stderr, "Error parsing config file!\n\r");
        xmlFreeDoc(doc);
        return;
    }

    xmlChar *prop;

    prop =  xmlGetProp(cur, (const xmlChar*)"version");

    printf("C: %s (%s)\n\r", cur->name, prop);
    
    xmlFree(prop);

    cur = cur->xmlChildrenNode;
    
    while(cur != NULL) {
	    printf("%s (%s)\n\r", cur->name, xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
        if(xmlStrcmp(cur->name, (const xmlChar *)"directories")==0)
            cur = cur->xmlChildrenNode;
        else
            cur = cur->next;
    }

    xmlFreeDoc(doc);
}
#endif
int main(int argc, char** argv) {
    //backup("t", "backup.bkp");

    //readConfig(argv[1]);
    Config c;
    if(argc < 2) {
        ERR("No arguments passed!\n\r");
        return 1;
    }

    if(!c.load(argv[1]))
        return 1;
    c.backupDirectories();
    return 0;
}

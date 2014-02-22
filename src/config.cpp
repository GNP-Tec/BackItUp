/**************************************************
 * FILENAME:        config.cpp
 * PROJECT:         BackItUp
 *
 * AUTHOR:          Philipp Doblhofer
 * WEB:             www.gnp-tec.net
 * START DATE:      2013-Dec-29
 *
 **************************************************
 * DESCRIPTION:
 * This is the class, which reads the configuration
 * file for the backuper (XML). Afterwards it will
 * call the FIterator, which saves the files and
 * directories as set in the config file.
 *************************************************/

#include "../inc/config.h"

// printf to stderr with negative return
#define ERRWR(x...)                 { fprintf(stderr, x); return false; }

#define XML_CMP_NODE(ptr, str)      (strcmp((const char*)ptr->name, str)==0)
#define XML_GET_TEXT(doc, ptr)      ((const char*)xmlNodeListGetString(doc, ptr->xmlChildrenNode, 1))

bool Config::Load(const char* cfg) {
    xmlDocPtr   doc = NULL;
    xmlNodePtr  cur = NULL;
    xmlChar*    prop = NULL;

    if(cfg == NULL || *cfg == '\0')
        ERRWR("Invalid filename!\n\r");

    doc = xmlParseFile(cfg);
    if(doc == NULL)
        ERRWR("Error parsing config file <%s>!\n\r", cfg);
    
    cur = xmlDocGetRootElement(doc);
    if(cur == NULL || strcmp((const char*)cur->name, "config") != 0)
        ERRWR("Error parsing config file <%s>!\n\r", cfg);

    // get configfile version
    prop = xmlGetProp(cur, (const xmlChar*)"version");
    if(prop == NULL)
        ERRWR("Error parsing config file <%s>!\n\r", cfg);
    version = strdup((const char*)prop);
    xmlFree(prop);  
    if(version == NULL)
        ERRWR("Error allocating memory!\n\r");

    // is <quiet> set?
    if(xmlHasProp(cur, (const xmlChar*)"quiet") == NULL)
        Log.addOutput(LogStdout, LogInfo, NULL, 0);
    
    while(cur != NULL) {
        // get next element
        if(cur->xmlChildrenNode != NULL) {
            cur=cur->xmlChildrenNode;
        } else if(cur->next != NULL) {
            cur=cur->next;
        } else {
            cur=cur->parent->next;
            if(cur == NULL)
                break;
        }

        if(cur->type == XML_ELEMENT_NODE) {
            #ifdef DEBUG
            Log.Log(LogInfo, "%s\n", (const char*)cur->name);
            #endif
            
            if(XML_CMP_NODE(cur, "log")) {
                if(!XML_CMP_NODE(cur->parent, "backup")) {
                    xmlFreeDoc(doc);
                    ERRWR("The <log> node must be inside of <backup>!\n");
                }
                if(XML_GET_TEXT(doc, cur) != NULL)
                    Log.addOutput(LogFile, LogInfo, XML_GET_TEXT(doc, cur), strlen(XML_GET_TEXT(doc, cur)));
            } else if(XML_CMP_NODE(cur, "destination")) {
                if(!XML_CMP_NODE(cur->parent, "backup")) {
                    xmlFreeDoc(doc);
                    ERRWR("The <destination> node must be inside of <backup>!\n");
                }
                if(XML_GET_TEXT(doc, cur) != NULL)
                    destination = strdup(XML_GET_TEXT(doc, cur));
            } else if(XML_CMP_NODE(cur, "type")) {
                if(!XML_CMP_NODE(cur->parent, "backup")) {
                    xmlFreeDoc(doc);
                    ERRWR("The <type> node must be inside of <backup>!\n");
                }
                if(XML_GET_TEXT(doc, cur) == NULL)
                    continue;
                if(strcmp(XML_GET_TEXT(doc, cur), "regular") == 0)
                    type = TYPE_REGULAR;
                if(strcmp(XML_GET_TEXT(doc, cur), "compressed") == 0)
                    type = TYPE_COMPRESSED;
            } else if(XML_CMP_NODE(cur, "directory")) {
                if(!XML_CMP_NODE(cur->parent, "directories")) {
                    xmlFreeDoc(doc);
                    ERRWR("A <directory> node must be inside of <directories>!\n");
                }
                if(XML_GET_TEXT(doc, cur) == NULL)
                    continue;
                dir.push_back(XML_GET_TEXT(doc, cur));
            } else if(XML_CMP_NODE(cur, "backup")) {
                //#warning delete
            } else if(XML_CMP_NODE(cur, "directories")) {
                //#warning delete
            } else {
                printf("Unhandled node <%s>\n\r", cur->name);
            }
            
        }
    }

    xmlFreeDoc(doc);
    return true;
}

bool Config::Validate(void) {
    if(version == NULL || *version == 0) {
        Log.Log(LogWarning, "Version is not set!\n");
        return false;
    }
    if(destination == NULL || *destination == 0) {
        Log.Log(LogWarning, "Destination is not set!\n");
        return false;
    }
    if(type == TYPE_UNSET) {
        Log.Log(LogWarning, "Type is not set!\n");
        return false;
    }
    if(dir.empty()) {
        Log.Log(LogWarning, "No backup directories are given!\n");
        return false;
    }
    return true;
}

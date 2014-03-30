/**************************************************
 * FILENAME:        BackItUp.cpp
 * PROJECT:         BackItUp
 *
 * AUTHOR:          Philipp Doblhofer
 * WEB:             www.gnp-tec.net
 * START DATE:      2014-Jan-18
 *
 **************************************************
 * DESCRIPTION:
 * "Glue-logic" class.
 *************************************************/

#include "../inc/BackItUp.h"
#include "../inc/Backup.h"
#include "../inc/regular.h"
#include "../inc/compressed.h"
#include <stdlib.h>
#include <string.h>	

#define ERR(x...)   {fprintf(stderr, x);}

#ifdef DEBUG
#define V_TYPE "- DEBUG"
#elif defined(PROFILING)
#define V_TYPE "- PROFILING"
#else
#define V_TYPE ""
#endif

Logger Log;

#warning TYPE_ENCRYPTED

void BackItUp::usage(char* pgm) {
    ERR("Usage: %s\n", pgm);
    ERR("\tbackup    <config>\t\tInitiate a backup as\n\t\t\t\t\tconfigured in a config file\n");
    ERR("!\tbackup    <backup>\t\tMake an incremential backup\n");
    ERR("!\trestore   <backup>\t\tRestore a backup\n");
    ERR("\tverify    <backup>\t\tVerifies a backup\n");
    ERR("\tgetconfig <backup>\t\tPrints the config of a backup\n");
    ERR("\tcheck     <config>\t\tValidate a config file\n");
    ERR("\tversion           \t\tGet the current version\n");
}

void BackItUp::version() {
    ERR("BackItUp v%i.%i %s\n\rCopyright (c) 2014 GNP-Tec.net, Philipp Doblhofer\n\r", V_MAJ, V_MIN, V_TYPE);
}

BackItUp::BackItUp(int argc, char** argv) {
    if(argc == 1) {
        usage(*argv);
        exit(0);
    }

    if(strncmp(argv[1], "backup", strlen("backup")+1)==0) {
        if(argc != 3) {
            usage(*argv);
            exit(1);
        }
        configPath = argv[2];

        if(strcmp(configPath + strlen(configPath) - 4, ".xml") == 0) {
            if(!c.Load(argv[2])) {
                Log.Log(LogError, "<%s> is an invalid config file!\n", argv[2]);
                exit(1);
            }
            if(!c.Validate()) {
                Log.Log(LogError, "<%s> is an invalid config file!\n", argv[2]);
                exit(1);
            }

            Backup *b = NULL;
            if(c.GetBackupType() == TYPE_REGULAR)
                b = new RegularBackup(this);
            else if(c.GetBackupType() == TYPE_COMPRESSED)
                b = new CompressedBackup(this);
            
            if(b==NULL) {
                Log.Log(LogError, "Error initialising backup!\n");
                exit(1);
            }

            if(!b->Initialize()) {
                Log.Log(LogError, "Error initialising backup!\n");
                if(!b->Finalize())
                    Log.Log(LogError, "Error finalizing backup!\n");
                exit(1);
            }
            while(c.IsNextBackupDirectory())
                b->addFolder(c.GetNextBackupDirectory());

            if(!b->Finalize()) {
                Log.Log(LogError, "Error finalizing backup!\n");
                exit(1);
            }
        } else {
            struct stat attr;
            if(lstat(configPath, &attr) < 0) {
                fprintf(stderr , "Error getting file information <%s>!\n\r", configPath);
                return ;
            }

            Backup *b = NULL;
            if((attr.st_mode & S_IFMT) == S_IFDIR)
                b = new RegularBackup(this);
            else if((attr.st_mode & S_IFMT) == S_IFREG)
                b = new CompressedBackup(this);

            if(b==NULL) {
                Log.Log(LogError, "Error initialising backup!\n");
                exit(1);
            }

            b->OpenBackup(argv[2]);
            char *buf = b->GetConfig();
            c.Load(buf, false);
            free(buf);
            b->Compare();
#warning CREATE INCREMENTIAL BACKUP
            b->CloseBackup();

            exit(0);
        }            
    } else if(strncmp(argv[1], "verify", strlen("verify")+1)==0) {
        Log.addOutput(LogStdout, LogInfo, NULL, 0);
        if(argc != 3) {
            usage(*argv);
            exit(1);
        }

        struct stat attr;
        if(lstat(argv[2], &attr)<0) {
            Log.Log(LogError, "Error getting file information <%s>!\n\r", argv[2]);
            return ;        
        }

        Backup *b = NULL;
        if((attr.st_mode & S_IFMT) == S_IFDIR) {
            b = new RegularBackup(this);
        } else {
            b = new CompressedBackup(this);
        }

        b->OpenBackup(argv[2]);
        char *buf = b->GetConfig();
        c.Load(buf, false);
        free(buf);
        b->Compare();
        b->CloseBackup();

        exit(0);
    } else if(strncmp(argv[1], "getconfig", strlen("getconfig")+1)==0) {
        Log.addOutput(LogStdout, LogInfo, NULL, 0);
        if(argc != 3) {
            usage(*argv);
            exit(1);
        }

        struct stat attr;
        if(lstat(argv[2], &attr)<0) {
            Log.Log(LogError, "Error getting file information <%s>!\n\r", argv[2]);
            return ;        
        }

        Backup *b = NULL;
        if((attr.st_mode & S_IFMT) == S_IFDIR) {
            b = new RegularBackup(this);
        } else {
            b = new CompressedBackup(this);
        }

        b->OpenBackup(argv[2]);
        b->PrintConfig();
        b->CloseBackup();

        exit(0);
    } else if(strncmp(argv[1], "version", strlen("version")+1)==0) {
        version();
        exit(0);
    } else if(strncmp(argv[1], "check", strlen("check")+1)==0) {
        if(argc != 3) {
            usage(*argv);
            exit(1);
        }
        if(!c.Load(argv[2])) {
            Log.Log(LogError, "<%s> is an invalid config file!\n", argv[2]);
            exit(1);
        }
        if(c.Validate()) {
            Log.Log(LogInfo, "<%s> is a valid config file!\n", argv[2]);
            exit(0);
        } else {
            Log.Log(LogError, "<%s> is an invalid config file!\n", argv[2]);
            exit(1);
        }  
    } else {
        usage(*argv);
        exit(1);
    }
}

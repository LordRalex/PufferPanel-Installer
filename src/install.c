#define _POSIX_C_SOURCE 2
#define _XOPEN_SOURCE 700
#include "headers/common.h"
#include "headers/install.h"
#include "headers/logging.h"
#include "headers/main.h"
#include "headers/tarread.h"
#include <dirent.h>
#include <mysql/mysql.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/*
 * Pointers to resources
 */
extern char _binary_resources_database_sql_start;
extern char _binary_resources_database_sql_end;
extern char _binary_resources_install_php_start;
extern char _binary_resources_install_php_end;
extern char _binary_resources_language_php_start;
extern char _binary_resources_language_php_end;
extern char _binary_resources_config_json_start;
extern char _binary_resources_config_json_end;
extern char _binary_resources_pufferpanel_tar_start;
extern char _binary_resources_pufferpanel_tar_end;

/*
 * Validate panel dependencies are installed
 */
bool validateDependencies() {
    bool phpInstalled = validateCommand("php");
    bool canInstall = true;

    if (phpInstalled) {
        logOut("PHP-CLI is installed\n");
    } else {
        logOut("PHP-CLI is not installed\n");
        canInstall = false;
    }

    if (validateCommand("git")) {
        logOut("Git is installed\n");
    } else {
        logOut("Git is not installed\n");
    }

    if (validateCommand("mysql")) {
        logOut("MySQL-Client is installed\n");
    } else {
        logOut("MySQL-Client is not installed\n");
        canInstall = false;
    }
    if (phpInstalled) {
        const char* exts[6] = {
            "curl",
            "hash",
            "openssl",
            "mcrypt",
            "pdo",
            "pdo_mysql"
        };
        int i = 0;
        char* cmd;
        FILE *ofp;

        while (i < 6) {
            char tempName[19] = "PufferPanel.XXXXXX";
            mkstemp(tempName);
            ofp = fopen(tempName, "w");
            fprintf(ofp, "<?php exit(extension_loaded(\"%s\") ? 0 : 1); ?>", exts[i]);
            fclose(ofp);
            cmd = concat(3, "php -f ", tempName, " 1>/dev/null 2>&1");
            if (system(cmd) == 0) {
                logOut("PHP-%s is installed\n", exts[i]);
            } else {
                logOut("PHP-%s is not installed\n", exts[i]);
                canInstall = false;
            }
            unlink(tempName);
            i++;
        }
    } else {
        logOut("Cannot validate PHP dependencies as PHP is not installed\n");
    }
    return canInstall;
}

/*
 * Validates command exists on the system
 */
bool validateCommand(char* command) {
    return system(concat(3, "type ", command, " 1>/dev/null 2>&1")) == 0;
}

bool extractPanel(const char* installPath, const char* installUser) {
    char tempName[23] = "pufferpanel.tar.XXXXXX";
    mkstemp(tempName);
    int offset = 0;
    FILE *out;
    out = fopen(tempName, "w");
    char buffer[64];
    int size = &_binary_resources_pufferpanel_tar_end - &_binary_resources_pufferpanel_tar_start;
    while (offset < size) {
        int counter = 0;
        while (counter < 64) {
            buffer[counter] = *(&_binary_resources_pufferpanel_tar_start + offset);
            counter++;
            offset++;
        }
        fwrite(buffer, 1, 64, out);
    }
    extract(tempName, installPath);
    unlink(tempName);
    return true;
}

bool finalizeInstall(const char* installPath, const char* installUser) {
    return true;
};
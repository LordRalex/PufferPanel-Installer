#define _POSIX_C_SOURCE 2
#define _XOPEN_SOURCE 700
#include "headers/common.h"
#include "headers/install.h"
#include "headers/logging.h"
#include "headers/main.h"
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
        canInstall = false;
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
    if (system(concat(3, "type ", command, " 1>/dev/null 2>&1")) == 0) {
        return true;
    } else {
        return false;
    }
}

bool buildLang(const char* path) {
    logOut("Building language files\n");
    DIR *dir;
    struct dirent *ent;
    char* fullPath = concat(2, path, "/app/languages/");
    logOutFile("Copying language.php to %s\n", fullPath);

    FILE *php;
    php = fopen(concat(2, fullPath, "language.php"), "w");
    fprintf(php, &_binary_resources_language_php_start);
    fclose(php);

    char* cmd = concat(3, "cd ", fullPath, " && php -f language.php ");
    bool success = true;

    if ((dir = opendir(concat(2, fullPath, "raw/"))) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            char* name = ent->d_name;
            if (isEqual(name, ".") || isEqual(name, "..")) {
                continue;
            }
            char lang[3];
            strncpy(lang, name, 2);
            lang[2] = 0;
            logOutFile("Building %s\n", lang);
            if (system(concat(2, cmd, lang)) != 0) {
                success = false;
            };
        }
        closedir(dir);
    }

    unlink(concat(2, fullPath, "language.php"));
    logOut("Building %s\n", success ? "successful" : "failed");
    return success;
}

bool buildConfig(const char* path) {
    char host[64] = "localhost";
    char database[11] = "pufferpanel";
    char username[64] = "root";
    char password[64];
    char temp[64];
    bool successful = false;

    logOutFile("Asking for config input, hiding this for security reasons\n");
    while (!successful) {
        printf("Enter MySQL host (%s): ", host);
        fgets(temp, 128, stdin);
        if (!isEqual(temp, "")) {
            strcpy(host, temp);
        }
        printf("Enter MySQL username (%s): ", username);
        fgets(temp, 128, stdin);
        if (!isEqual(temp, "")) {
            strcpy(username, temp);
        }
        printf("Enter MySQL password: ");
        fgets(password, 128, stdin);

        if (system(concat(7, "mysql -h ", host, " -u ", username, " -p", password, " 1>/dev/null 2>&1")) == 0) {
            successful = true;
        }
    }

    FILE* config;
    config = fopen(concat(2, path, "/install.json"), "w");
    fprintf(config, &_binary_resources_config_json_start, host, database, username, password);
    fclose(config);
    return true;
}

bool installComposer(const char* installPath) {
    logOut("Setting up composer\n");
    logOut("Downloading... ");
    logOutFile("\n");
    FILE* process;
    char path[1024];
    process = popen(concat(2, "php -r \"readfile('https://getcomposer.org/installer');\" | php -- --install-dir=", installPath, " 2>&1"), "r");
    while (fgets(path, sizeof (path) - 1, process) != NULL) {
        logOutFile("%s\n", path);
    }
    if (pclose(process) != 0) {
        logOut("failed\n");
        return false;
    }
    logOut("done\n");
    logOut("Installing packages... ");
    logOutFile("\n");
    process = popen(concat(3, "cd ", installPath, " && php composer.phar install 2>&1"), "r");
    while (fgets(path, sizeof (path) - 1, process) != NULL) {
        logOutFile("%s\n", path);
    }
    if (pclose(process) != 0) {
        logOut("failed\n");
        return false;
    }
    logOut("done\n");
    return true;
}

bool installSQL(const char* path) {
    return true;
};

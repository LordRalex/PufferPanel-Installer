/*
 * File:   main.c
 * Author: Lord_Ralex
 *
 * Created on April 19, 2015, 11:57 PM
 */
#define _POSIX_C_SOURCE 2
#define _XOPEN_SOURCE 700
#include "main.h"
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

FILE *logFile;

/*
 * Entry point
 */
int main(int argc, char** argv) {

    const char* ppversion = NULL;
    bool useDev = false;
    bool testInstall = false;
    char* installPath = "PufferPanel";
    char* installUser = "apache";
    bool doLangOnly = false;
    bool doConfigOnly = false;

    /*
     * Process command line arguments
     */
    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-d") == 0) {
            useDev = true;
        } else if (strcmp(argv[i], "-u") == 0) {
            if (i + 1 < argc - 1) {
                printUsage(argv[0]);
                return (EXIT_FAILURE);
            } else {
                i++;
                installUser = argv[i];
            }
        } else if (strcmp(argv[i], "-t") == 0) {
            testInstall = true;
        } else if (strcmp(argv[i], "-l") == 0) {
            doLangOnly = true;
        } else if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 < argc - 1) {
                printUsage(argv[0]);
                return (EXIT_FAILURE);
            } else {
                i++;
                installPath = argv[i];
            }
        } else if (strcmp(argv[i], "-v") == 0) {
            printf("PufferPanel Installer - Version C-0.0.1\n");
            return (EXIT_SUCCESS);
        } else if (strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            return (EXIT_SUCCESS);
        } else if (strcmp(argv[i], "-c") == 0) {
            doConfigOnly = true;
        } else {
            printUsage(argv[0]);
            return (EXIT_FAILURE);
        }
        i++;
    }

    logFile = fopen("ppinstaller.log", "w");
    logOut("PufferPanel Installer - Version C-0.0.1\n");

    fprintf(logFile, "Using dev build: %s\n", useDev ? "true" : "false");
    fprintf(logFile, "Testing install: %s\n", testInstall ? "true" : "false");
    fprintf(logFile, "Building languages only: %s\n", doLangOnly ? "true" : "false");
    fprintf(logFile, "Building config only: %s\n", doConfigOnly ? "true" : "false");
    fprintf(logFile, "Install path: %s\n", installPath);
    fprintf(logFile, "Install user: %s\n", installUser);

    if (doLangOnly) {
        return (buildLang(installPath) ? EXIT_SUCCESS : EXIT_FAILURE);
    } else if (doConfigOnly) {
        return (buildConfig(installPath) ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    logOut("-----\n");
    bool result = validateDependencies();
    logOut("-----\n");
    if (result) {
        logOut("Dependencies are met, installing.\n");
        if (testInstall) {
            logOut("Halting installation due to test flag being present.\n");
            return (EXIT_SUCCESS);
        }
    } else {
        logOut("Halting installation as dependencies are not met. Please fix this before continuing.\n");
        return (EXIT_FAILURE);
    }

    logOut("-----\nCloning panel to %s\n", installPath);
    FILE *process;
    char path[1024];
    process = popen(concat(3, "git clone https://github.com/PufferPanel/PufferPanel ", installPath, " 2>&1"), "r");
    while (fgets(path, sizeof (path) - 1, process) != NULL) {
        fprintf(logFile, "%s\n", path);
    }
    if (pclose(process) != 0) {
        logOut("Cloning failed\n");
        return (EXIT_FAILURE);
    }

    logOut("-----\nChecking out correct version\n");
    if (ppversion == NULL) {
        logOut("- Dev install\n");
    } else {
        logOut("- %s\n", ppversion);
    }

    logOut("-----\n");
    if (!buildLang(installPath)) {
        return (EXIT_FAILURE);
    }

    logOut("-----\n");
    if (!buildConfig(installPath)) {
        return (EXIT_FAILURE);
    }

    logOut("-----\n");
    if (!installComposer(installPath)) {
        return (EXIT_FAILURE);
    }

    logOut("-----\n");


    logOut("-----\n");
    logOut("Installation complete, thank you for choosing PufferPanel!\n");
    return (EXIT_SUCCESS);
}

/*
 * Prints the usage line for the installer, reduces repetition
 */
void printUsage(char* file) {
    fprintf(stderr, "usage: %s [-dtl] [-v] [-u user] [-p path]\n", file);
}

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

/**
 * Log to stdout and logFile
 */
void logOut(char const *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    va_start(ap, fmt);
    vfprintf(logFile, fmt, ap);
    va_end(ap);
}

char* concat(int size, ...) {
    va_list ap;
    va_start(ap, size);
    int i = 0;
    int length = 1;
    int s;
    while (i < size) {
        char* arg = va_arg(ap, char*);
        length += strlen(arg);
        i++;
    }
    va_end(ap);
    va_start(ap, size);
    char* newBuffer = (char*) malloc(length);
    i = 1;
    strcpy(newBuffer, va_arg(ap, char*));
    while (i < size) {
        strcat(newBuffer, va_arg(ap, char*));
        i++;
    }
    va_end(ap);
    return newBuffer;
}

/*bool buildLang(const char* path) {
}*/

bool buildLang(const char* path) {
    logOut("Building language files\n");
    DIR *dir;
    struct dirent *ent;
    char* fullPath = concat(2, path, "/app/languages/");
    fprintf(logFile, "Copying language.php to %s\n", fullPath);

    FILE *php;
    php = fopen(concat(2, fullPath, "language.php"), "w");
    fprintf(php, &_binary_resources_language_php_start);
    fclose(php);

    char* cmd = concat(3, "cd ", fullPath, " && php -f language.php ");
    bool success = true;

    if ((dir = opendir(concat(2, fullPath, "raw/"))) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            char* name = ent->d_name;
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
                continue;
            }
            char lang[3];
            strncpy(lang, name, 2);
            lang[2] = 0;
            fprintf(logFile, "Building %s\n", lang);
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

bool installComposer(const char* installPath) {
    logOut("Setting up composer\n");
    logOut("Downloading... ");
    fprintf(logFile, "\n");
    FILE* process;
    char path[1024];
    process = popen(concat(2, "php -r \"readfile('https://getcomposer.org/installer');\" | php -- --install-dir=", installPath, " 2>&1"), "r");
    while (fgets(path, sizeof (path) - 1, process) != NULL) {
        fprintf(logFile, "%s\n", path);
    }
    if (pclose(process) != 0) {
        logOut("failed\n");
        return false;
    }
    logOut("done\n");
    logOut("Installing packages... ");
    fprintf(logFile, "\n");
    process = popen(concat(3, "cd ", installPath, " && php composer.phar install 2>&1"), "r");
    while (fgets(path, sizeof (path) - 1, process) != NULL) {
        fprintf(logFile, "%s\n", path);
    }
    if (pclose(process) != 0) {
        logOut("failed\n");
        return false;
    }
    logOut("done\n");
    return true;
}

bool buildConfig(const char* path) {
    char host[128];
    char database[11] = "pufferpanel";
    char username[128];
    char password[128];

    fprintf(logFile, "Asking for config input, hiding this for security reasons\n");
    printf("Enter MySQL host: ");
    fgets(host, 1024, stdin);
    printf("Enter MySQL username: ");
    fgets(host, 1024, stdin);
    printf("Enter MySQL password: ");
    fgets(host, 1024, stdin);

    FILE* config;
    config = fopen(concat(2, path, "/config.json"), "w");
    fprintf(config, &_binary_resources_config_json_start, host, database, username, password);
    fclose(config);
    return true;
}

bool installSQL(const char* path) {
    return true;
};
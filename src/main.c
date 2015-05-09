/*
 * File:   main.c
 * Author: Lord_Ralex
 *
 * Created on April 19, 2015, 11:57 PM
 */
#define _POSIX_C_SOURCE 2
#define _XOPEN_SOURCE 700
#include "common.h"
#include "install.h"
#include "logging.h"
#include "main.h"
#include <dirent.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/*
 * Entry point
 */
int main(int argc, char** argv) {

    const char* ppversion = NULL;
    bool testInstall = false;
    char* installPath = "PufferPanel";
    char* installUser;
    char* distro = getDistro();
    if (isEqual(distro, "ubuntu")) {
        installUser = "www-data";
    } else if (isEqual(distro, "debian")) {
        installUser = "www-data";
    } else if (isEqual(distro, "unknown")) {
        installUser = "apache";
    } else {
        installUser = "apache";
    }

    bool doLangOnly = false;
    bool doConfigOnly = false;

    /*
     * Process command line arguments
     */
    int i = 1;
    while (i < argc) {
        if (isEqual(argv[i], "-u")) {
            if (i + 1 < argc - 1) {
                printUsage(argv[0]);
                return (EXIT_FAILURE);
            } else {
                i++;
                installUser = argv[i];
            }
        } else if (isEqual(argv[i], "-t")) {
            testInstall = true;
        } else if (isEqual(argv[i], "-l")) {
            doLangOnly = true;
        } else if (isEqual(argv[i], "-v")) {
            printf("PufferPanel Installer - Version C-0.0.1 (%s)\n", ppversion);
            return (EXIT_SUCCESS);
        } else if (isEqual(argv[i], "-h")) {
            printUsage(argv[0]);
            return (EXIT_SUCCESS);
        } else if (isEqual(argv[i], "-c")) {
            doConfigOnly = true;
        } else {
            printUsage(argv[0]);
            return (EXIT_FAILURE);
        }
        i++;
    }

    if (!testInstall) {
        char temp[512];
        printf("Enter install path (%s)", installPath);
        fgets(temp, 512, stdin);
        if (!isEqual(temp, "")) {
            strcpy(installPath, temp);
        }
    }

    startLogging();
    int returnCode = innerMain(testInstall, doLangOnly, doConfigOnly, installPath, installUser, ppversion);
    closeLogging();
    return returnCode;
}

/*
 * Prints the usage line for the installer, reduces repetition
 */
void printUsage(char* file) {
    fprintf(stderr, "usage: %s [-dtl] [-v] [-u user] [-p path]\n", file);
}

int innerMain(bool testInstall, bool doLangOnly, bool doConfigOnly, char* installPath, char* installUser, const char* ppversion) {
    logOut("PufferPanel Installer - Version C-0.0.1 (%s)\n", ppversion);
    logOutFile("Testing install: %s\n", testInstall ? "true" : "false");
    logOutFile("Building languages only: %s\n", doLangOnly ? "true" : "false");
    logOutFile("Building config only: %s\n", doConfigOnly ? "true" : "false");
    logOutFile("Install path: %s\n", installPath);
    logOutFile("Install user: %s\n", installUser);

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
        logOutFile("%s\n", path);
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
    if (!installComposer(installPath)) {
        return (EXIT_FAILURE);
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
    if (!installSQL(installPath)) {
        return (EXIT_FAILURE);
    }

    logOut("-----\n");
    logOut("Installation complete, thank you for choosing PufferPanel!\n");


    return (EXIT_SUCCESS);
}
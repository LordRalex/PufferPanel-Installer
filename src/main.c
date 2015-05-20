/*
 * File:   main.c
 * Author: Lord_Ralex
 *
 * Created on April 19, 2015, 11:57 PM
 */
#define _POSIX_C_SOURCE 2
#define _XOPEN_SOURCE 700
#define INSTALLERVERSION "0.0.1"
#define PPVERSION "0.8.1-beta"
#include "headers/common.h"
#include "headers/install.h"
#include "headers/logging.h"
#include "headers/main.h"
#include <dirent.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/*
 * Pointers to resources
 */
extern char _binary_pufferpanel_tar_start;
extern char _binary_pufferpanel_tar_end;

/*
 * Entry point
 */
int main(int argc, char** argv) {
    bool testInstall = false;
    char* installPath = "/srv/PufferPanel";
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
        } else if (isEqual(argv[i], "-v")) {
            printf("PufferPanel Installer - Version C-%s (%s)\n", INSTALLERVERSION, PPVERSION);
            return (EXIT_SUCCESS);
        } else if (isEqual(argv[i], "-h")) {
            printUsage(argv[0]);
            return (EXIT_SUCCESS);
        } else {
            printUsage(argv[0]);
            return (EXIT_FAILURE);
        }
        i++;
    }

    startLogging();
    int returnCode = innerMain(testInstall, installPath, installUser);
    closeLogging();
    return returnCode;
}

/*
 * Prints the usage line for the installer, reduces repetition
 */
void printUsage(char* file) {
    fprintf(stderr, "usage: %s [-t] [-v] [-u user]\n", file);
}

int innerMain(bool testInstall, char* installPath, char* installUser) {
    logOut("PufferPanel Installer - Version C-%s (%s)\n", INSTALLERVERSION, PPVERSION);
    logOutFile("Testing install: %s\n", testInstall ? "true" : "false");
    logOutFile("Install path: %s\n", installPath);
    logOutFile("Install user: %s\n", installUser);

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

    logOut("-----\n");
    logOut("Extracting panel\n");
    if (!extractPanel(installPath, installUser)) {
        return (EXIT_FAILURE);
    }

    logOut("-----\n");
    if (!finalizeInstall(installPath, installUser)) {
        return (EXIT_FAILURE);
    }

    logOut("-----\n");
    logOut("Installation complete, thank you for choosing PufferPanel!\n");

    return (EXIT_SUCCESS);
}
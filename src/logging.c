/*
 * File:   logging.c
 * Author: Joshua
 *
 * Created on May 8, 2015, 4:35 PM
 */

#include "headers/logging.h"
#include <dirent.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

FILE* logFile = NULL;

void startLogging() {
    logFile = fopen("ppinstaller.log", "w");
}

void logOut(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    va_start(ap, fmt);
    vfprintf(logFile, fmt, ap);
    va_end(ap);
}

void logOutFile(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(logFile, fmt, ap);
    va_end(ap);
}

void closeLogging() {
    fclose(logFile);
}
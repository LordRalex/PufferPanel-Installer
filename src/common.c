/*
 * File:   common.c
 * Author: Lord_Ralex
 */
#define _POSIX_C_SOURCE 2
#define _XOPEN_SOURCE 700
#include "common.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

char* getDistro() {
    char* distro;
    char output[64];
    FILE* process;
    process = popen(". /etc/os-release 2>/dev/null; echo $ID 2>/dev/null", "r");
    while (fgets(output, sizeof (output) - 1, process) != NULL) {
        distro = output;
    }
    pclose(process);
    if (strcmp(distro, "")) {
        distro = "unknown";
    }
    return distro;
}

bool isEqual(const char* str1, const char* str2) {
    return strcmp(str1, str2) == 0;
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
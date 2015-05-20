/* 
 * File:   main.h
 * Author: Joshua
 *
 * Created on April 22, 2015, 9:14 PM
 */

#ifndef MAIN_H
#define	MAIN_H

#include <stdarg.h>
#include <stdbool.h>

void printUsage(char* file);
int innerMain(bool testInstall, char* installPath, char* installUser);

#endif


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
bool validateDependencies();
bool validateCommand(char* command);
void logOut(char const *fmt, ...);
bool cloneRepo();
char* concat(int size, ...);
bool buildLang(const char* folder);
bool buildConfig(const char* folder);
bool installComposer(const char* path);
bool installSQL(const char* path);

#endif


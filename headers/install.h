/* 
 * File:   install.h
 * Author: Joshua
 *
 * Created on May 8, 2015, 4:50 PM
 */

#ifndef INSTALL_H
#define	INSTALL_H

bool validateDependencies();
bool validateCommand(char* command);
bool cloneRepo();
bool buildLang(const char* folder);
bool buildConfig(const char* folder);
bool installComposer(const char* path);
bool installSQL(const char* path);

#endif	/* INSTALL_H */


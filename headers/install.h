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
bool extractPanel(const char* installPath, const char* installUser);
bool finalizeInstall(const char* installPath, const char* installUser);

#endif	/* INSTALL_H */


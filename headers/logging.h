/* 
 * File:   logging.h
 * Author: Joshua
 *
 * Created on May 8, 2015, 4:35 PM
 */

#ifndef LOGGING_H
#define	LOGGING_H

void startLogging();
void logOut(char const *fmt, ...);
void logOutFile(char const *fmt, ...);
void closeLogging();

#endif	/* LOGGING_H */


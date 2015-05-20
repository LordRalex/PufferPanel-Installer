#include "headers/common.h"
#include "headers/logging.h"
#include "headers/tarread.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct stat Stat;

#ifndef lint
/* Prevent over-aggressive optimizers from eliminating ID string */
const char jlss_id_mkpath_c[] = "@(#)$Id: mkpath.c,v 1.13 2012/07/15 00:40:37 jleffler Exp $";
#endif /* lint */

bool isEmpty(char* block, int size);
int mkpath(const char *path, mode_t mode);
int do_mkdir(const char *path, mode_t mode);
char *strdup(const char *s);

void extract(const char* file, const char* output) {
    char buffer[512];
    char name[512];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char devnull[12];

    FILE *in;
    in = fopen(file, "r");

    //each file
    int bufferCount;
    int fileSize;
    int done = false;
    while (fread(name, 1, 100, in) != 0 && !done) {
        fread(mode, 1, 8, in);
        fread(uid, 1, 8, in);
        fread(gid, 1, 8, in);
        fread(size, 1, 12, in);
        fread(mtime, 1, 12, in);
        fread(chksum, 1, 8, in);
        fread(typeflag, 1, 1, in);
        fread(linkname, 1, 100, in);
        fread(magic, 1, 6, in);
        fread(version, 1, 2, in);
        fread(uname, 1, 32, in);
        fread(gname, 1, 32, in);
        fread(devmajor, 1, 8, in);
        fread(devminor, 1, 8, in);
        fread(prefix, 1, 155, in);
        fread(devnull, 1, 12, in);

        bool isEOL = false;
        if (isEmpty(name, 100)) {
            logOutFile("Name flag is all 0's, possibly EOL\n");
            isEOL = true;
        } else if (isEqual(name, "././@LongLink")) {
            logOutFile("Detected a long file name, adjusting\n");

            fread(name, 1, 512, in);
            char old[100];
            fread(old, 1, 100, in);
            fread(mode, 1, 8, in);
            fread(uid, 1, 8, in);
            fread(gid, 1, 8, in);
            fread(size, 1, 12, in);
            fread(mtime, 1, 12, in);
            fread(chksum, 1, 8, in);
            fread(typeflag, 1, 1, in);
            fread(linkname, 1, 100, in);
            fread(magic, 1, 6, in);
            fread(version, 1, 2, in);
            fread(uname, 1, 32, in);
            fread(gname, 1, 32, in);
            fread(devmajor, 1, 8, in);
            fread(devminor, 1, 8, in);
            fread(prefix, 1, 155, in);
            fread(devnull, 1, 12, in);

        }
        logOutFile("Extracting %s\n", name);
        fileSize = strtol(size, NULL, 8);
        bufferCount = fileSize > 0 ? ((fileSize - 1) / 512) + 1 : 0;
        if (isEOL && bufferCount <= 0) {
            fread(buffer, 1, 512, in);
            if (isEOL && isEmpty(buffer, 512)) {
                logOutFile("EOL reached");
                done = true;
            }
        }
        if (bufferCount == 0) {
            //object is a folder
            mkpath(concat(3, output, "/", name), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        } else {
            //object is a file
            FILE* out;
            out = fopen(concat(3, output, "/", name), "w");
            while (bufferCount-- > 0) {
                fread(buffer, 1, 512, in);
                fwrite(buffer, 1, 512, out);
            }
            fclose(out);
        }
        logOutFile("--\n");
    }

    fclose(in);
}

bool isEmpty(char* block, int size) {
    while (size-- > 0) {
        if (block[size] != 0) {
            return false;
        }
    }
    return true;
}

/*
 * Following methods are from http://stackoverflow.com/a/675193/2253604 *
 */
int do_mkdir(const char *path, mode_t mode) {
    Stat st;
    int status = 0;

    if (stat(path, &st) != 0) {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            status = -1;
    } else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        status = -1;
    }

    return (status);
}

/**
 ** mkpath - ensure all directories in path exist
 ** Algorithm takes the pessimistic view and works top-down to ensure
 ** each directory in path exists, rather than optimistically creating
 ** the last element and working backwards.
 */
int mkpath(const char *path, mode_t mode) {
    char *pp;
    char *sp;
    int status;
    char *copypath = strdup(path);

    status = 0;
    pp = copypath;
    while (status == 0 && (sp = strchr(pp, '/')) != 0) {
        if (sp != pp) {
            /* Neither root nor double slash in path */
            *sp = '\0';
            status = do_mkdir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (status == 0)
        status = do_mkdir(path, mode);
    free(copypath);
    return (status);
}

char *strdup(const char *s) {
    char *d = malloc(strlen(s) + 1); // Space for length plus nul
    if (d == NULL) return NULL; // No memory
    strcpy(d, s); // Copy the characters
    return d; // Return the new string
}
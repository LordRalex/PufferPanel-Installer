#include "headers/tarread.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

bool isEmpty(char* block, int size);

void extract(const char* file, const char* output) {
    char buffer[512];
    char name[100];
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
    //clear header
    int count = 1;
    while (count-- > 0) {
        fread(buffer, 1, 512, in);
    }
    //folder start
    fread(buffer, 1, 512, in);

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
            printf("Name flag is all 0's, possibly EOL");
            isEOL = true;
        }
        printf("Reading file head: %s\n", name);
        fileSize = strtol(size, NULL, 8);
        bufferCount = fileSize > 0 ? (fileSize / 512) + 1 : 0;
        printf("Containers: %i\n", bufferCount);
        if (isEOL && bufferCount == 0) {
            fread(buffer, 1, 512, in);
            if (isEOL && isEmpty(buffer, 512)) {
                printf("EOL reached");
                done = true;
            }
        }
        if (bufferCount == 0) {
            //object is a folder
            mkdir(name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        } else {
            //object is a file
            FILE* out;
            out = fopen(name, "w");
            //read data
            while (bufferCount-- > 0) {
                fread(buffer, 1, 512, in);
                fwrite(buffer, 1, 512, out);
            }
            fclose(out);
        }
        printf("\n------------\n");
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
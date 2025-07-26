#include "read_file.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "log.h"

FILE *fp = NULL;

void init_fget(char *filename) {
    fp = fopen(filename, "r");
    if (fp == NULL)
        PERROR("fopen");
}

uint64_t fget_line(char **line) {
    uint64_t len = 0;
    uint64_t readsz;
    readsz = getline(line, &len, fp);
    if (readsz == (uint64_t)-1)
        return -1;
    (*line)[readsz - 1] = '\0';
    return readsz;
}

void free_fget(char *line) {
    free(line);
    fclose(fp);
}

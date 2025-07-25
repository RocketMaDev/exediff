#ifndef READ_FILE
#define READ_FILE

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

void init_fget (char *filename);

uint64_t fget_line (char **line);

#endif

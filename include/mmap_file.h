#ifndef MMAP
#define MMAP

#include <stdint.h>

typedef struct {
    uint8_t *file_buf;
    uint64_t file_len;
} mmap_file;

mmap_file *init_mmap_file(char *filename);

void free_mmap(mmap_file *file);

mmap_file *init_mmap_anoy(uint64_t len);

#endif

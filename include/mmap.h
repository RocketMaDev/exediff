#ifndef MMAP
#define MMAP

#include <stdint.h>

typedef struct
{
  char *content;
  uint8_t file_len;
} mmap_cont;

mmap_cont *mmap_file (char *filename);

void free_mmap (mmap_cont *file);

mmap_cont *mmap_anoy (uint32_t len);

#endif

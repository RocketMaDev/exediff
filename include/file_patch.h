#ifndef FILE_PATCH
#define FILE_PATCH

#include <stdbool.h>
#include <stdint.h>

bool init_patch(uint32_t filesz, char *patch_to);

void copy_until_hunk(uint64_t patch_from_addr, uint64_t patch_to_addr);

void replace_hunk(uint64_t patch_from_addr, uint64_t patch_to_addr,
                  char patch_from_bytes[], char patch_to_bytes[], uint64_t patch_from_len,
                  uint64_t patch_to_len, uint64_t filesz);

void free_save_file();

#endif

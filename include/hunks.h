#ifndef HUNKS
#define HUNKS

#include <stdint.h>
uint32_t hunk_patch_from_addr (char *line);

uint32_t hunk_patch_from_len (char *line);

uint32_t hunk_patch_to_addr (char *line);

uint32_t hunk_patch_to_len (char *line);

#endif

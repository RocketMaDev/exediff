#ifndef ASM
#define ASM

#include <stdint.h>

uint64_t assemble (char *code, uint8_t **encode);

void free_asm (uint8_t *encode);

#endif

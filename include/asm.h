#ifndef ASM
#define ASM

#include <stdint.h>
uint8_t *assemble (char *code);

void free_asm (uint8_t *encode);

#endif

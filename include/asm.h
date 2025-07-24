#ifndef ASM
#define ASM

#include <stdint.h>

typedef struct
{
  uint8_t *encode;
  uint64_t len;
} asm_code;

asm_code *assemble (char *code);

void free_asm (asm_code *encode);

#endif

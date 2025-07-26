#ifndef DISASM
#define DISASM

#include <stdint.h>

#define DISASM_CODE_LEN 0x40
typedef struct {
    char disasm_code[DISASM_CODE_LEN];
    uint32_t addr;
} disasm_line;

char *disassemble(char *to_disasm, uint32_t code_len);

void free_disasm(disasm_line *line);

#endif

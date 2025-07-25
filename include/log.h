#ifndef LOG
#define LOG

#include <stdio.h>
#include <stdlib.h>

#define PERROR(s)                                                             \
  {                                                                           \
    perror (s);                                                               \
    exit (-1);                                                                \
  }

#define PEXIT(s)                                                              \
  {                                                                           \
    puts (s);                                                                 \
    exit (-1);                                                                \
  }

#define ELF_INIT "ELF library initialization failed"
#define ELF_BEGIN "ELF begin failed"
#define NOT_ELF "not an ELF file"
#define ELF_GETPHDRNUM "elf_getphdrnum failed"
#define GELF_GETPHDR "gelf_getphdr"

#define CS_OPEN "cs_open failed"
#define FAIL_DISASM "failed to disasm"

#define KS_OPEN "ks_open failed"
#define FAIL_ASM "failed to asm"

#define INVALID_PATCH "invalid patch file"
#define NO_TARGET_FILE "target file not speciefied"
#define INVALID_ADDR "invalid addr"
#define NO_TARGET_ADDR "target addr not speciefied"
#define NO_PATCH_BYTE "target byte not speciefied"
#define INVALID_PATCH_BYTE "invalid patch byte"

#endif

#include "asm.h"
#include "log.h"
#include <keystone/keystone.h>
#include <stddef.h>
#include <stdint.h>

asm_code
assemble (char *code)
{
  ks_engine *ks;
  ks_err err;
  asm_code result;
  uint64_t count;

  err = ks_open (KS_ARCH_X86, KS_MODE_64, &ks);
  if (err != KS_ERR_OK)
    PEXIT (KS_OPEN);

  if (ks_asm (ks, code, 0, &(result.encode), &(result.len), &count)
      != KS_ERR_OK)
    PEXIT (FAIL_ASM);

  ks_close (ks);
  return result;
}

void
free_asm (uint8_t *encode)
{
  ks_free (encode);
}

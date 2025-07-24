#include "log.h"
#include <keystone/keystone.h>
#include <stddef.h>

uint8_t *
assemble (char *code)
{
  ks_engine *ks;
  ks_err err;
  unsigned char *encode;
  size_t size;
  size_t count;

  err = ks_open (KS_ARCH_X86, KS_MODE_64, &ks);
  if (err != KS_ERR_OK)
    PEXIT (KS_OPEN);

  if (ks_asm (ks, code, 0, &encode, &size, &count) != KS_ERR_OK)
    PEXIT (FAIL_ASM);

  ks_close (ks);
  return encode;
}

void
free_asm (uint8_t *encode)
{
  ks_free (encode);
}

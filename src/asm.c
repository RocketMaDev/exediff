#include "asm.h"

#include <keystone/keystone.h>
#include <stddef.h>
#include <stdint.h>

#include "log.h"

uint64_t assemble(char *code, uint8_t **encode) {
    ks_engine *ks;
    ks_err err;
    uint64_t len;
    uint64_t count;

    err = ks_open(KS_ARCH_X86, KS_MODE_64, &ks);
    if (err != KS_ERR_OK)
        PEXIT(KS_OPEN);

    if (ks_asm(ks, code, 0, encode, &len, &count) != KS_ERR_OK)
        len = -1;

    ks_close(ks);
    return len;
}

void free_asm(uint8_t *encode) {
    ks_free(encode);
}

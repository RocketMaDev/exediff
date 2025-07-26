#include "bytes_or_asm.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"
#include "log.h"

bool try_asm(char *line, uint32_t *idx, char bytes[]) {
    uint64_t len;
    len = assemble(line, (uint8_t **)&bytes);
    if (len == (uint64_t)-1)
        return false;

    *idx += len;
    return true;
}

bool try_bytes(char *line, uint32_t *idx, char bytes[]) {
    char *current = line;
    char *end;

    do {
        uint64_t value = strtoull(current, &end, 16);
        if (current == end)
            return false;

        bytes[(*idx)++] = (char)value;
        current = end;
    } while ((current = strchr(current, ' ')) != NULL);

    return true;
}

void bytes_or_asm(char *line, uint32_t *idx, char bytes[]) {
    if (try_bytes(line, idx, bytes))
        return;
    else if (try_asm(line, idx, bytes))
        return;
    PEXIT(INVALID_PATCH_BYTE)
}

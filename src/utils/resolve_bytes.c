#include "resolve_bytes.h"
#include "log.h"
#include <stdint.h>
#include <stdlib.h>

void
resolve_bytes (char *line, uint32_t *idx, char bytes[])
{
  char *end;

  bytes[(*idx)++] = strtoll (line, &end, 0x10);
  if (*end != ' ')
    PEXIT (INVALID_PATCH_BYTE);

  while (*idx < LINE_LEN)
    {
      bytes[(*idx)++] = strtoll (line, &end, 0x10);
      if (*end != ' ')
        break;
      line += 3;
    }
}

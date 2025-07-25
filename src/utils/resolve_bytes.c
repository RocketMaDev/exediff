#include "resolve_bytes.h"
#include "log.h"
#include <stdint.h>
#include <stdlib.h>

void
resolve_bytes (char *line, uint32_t *idx, char bytes[])
{
  char *end;

  uint64_t byte = strtoll (line, &end, 0x10);
  if (line == end)
    PEXIT (INVALID_PATCH_BYTE);
  bytes[(*idx)++] = byte;

  line += 3;

  while (*idx < LINE_LEN)
    {
      uint64_t byte = strtoll (line, &end, 0x10);
      if (*end != ' ')
        break;
      bytes[(*idx)++] = byte;
      line += 3;
    }
}

#include "bytes_or_asm.h"
#include "asm.h"
#include "log.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

bool
try_asm (char *line, uint32_t *idx, char bytes[])
{
  uint64_t len;
  len = assemble (line, (uint8_t **)&bytes);
  if (len == (uint64_t)-1)
    return false;

  *idx += len;
  return true;
}

bool
try_bytes (char *line, uint32_t *idx, char bytes[])
{
  char *end;

  uint64_t byte = strtoll (line, &end, 0x10);
  if (line == end)
    return false;
  bytes[(*idx)++] = byte;
  line += 3;

  while (*idx < LINE_LEN)
    {
      byte = strtoll (line, &end, 0x10);
      if (*end != ' ')
        break;
      bytes[(*idx)++] = byte;
      line += 3;
    }
  return true;
}

void
bytes_or_asm (char *line, uint32_t *idx, char bytes[])
{
  if (try_bytes (line, idx, bytes))
    return;
  else if (try_asm (line, idx, bytes))
    return;
  PEXIT (INVALID_PATCH_BYTE)
}

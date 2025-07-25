#include "log.h"
#include <stdint.h>
#include <string.h>

char *end = NULL;

uint32_t
hunk_patch_from_addr (char *line)
{
  uint32_t patch_from_addr = strtoull (line, &end, 0);
  if (line == end)
    PEXIT (INVALID_ADDR);
  return patch_from_addr;
}

uint32_t
hunk_patch_from_len (char *line)
{
  line = strchr (end, ',') + 1;
  end = line;
  uint32_t patch_from_len = strtoull (line, &end, 0);
  if (line == end)
    PEXIT (INVALID_CTX_LEN);
  return patch_from_len;
}

uint32_t
hunk_patch_to_addr (char *line)
{
  line = strchr (end, '+') + 1;
  end = line;
  uint32_t new_addr = strtoull (line, &end, 0);
  if (line == end)
    PEXIT (INVALID_ADDR);
  return new_addr;
}

uint32_t
hunk_patch_to_len (char *line)
{
  line = strchr (end, ',') + 1;
  end = line;
  uint32_t patch_to_len = strtoull (line, &end, 0);
  if (line == end)
    PEXIT (INVALID_CTX_LEN);
  return patch_to_len;
}

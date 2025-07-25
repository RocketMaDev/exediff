#include "file_patch.h"
#include "log.h"
#include "mmap.h"
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

mmap_file *patch_to = NULL;
mmap_file *patch_from = NULL;
uint32_t patch_to_idx = 0;
uint32_t patch_from_idx = 0;

char *patch_to_name = NULL;

bool
init_patch (uint32_t filesz, char *file_name)
{
  patch_to = mmap_anoy (filesz + (0x2000 - (filesz % 0x1000)));
  patch_from = init_mmap_file (file_name);

  patch_to_name = file_name;
  return true;
}

void
copy_until_hunk (uint32_t copy_until)
{
  memcpy (patch_to->file_buf + patch_to_idx,
          patch_from->file_buf + patch_from_idx, copy_until - patch_to_idx);
  patch_from_idx += copy_until;
  patch_to_idx += copy_until;
}

void
replace_hunk (uint64_t patch_from_addr, uint64_t patch_to_addr,
              char patch_from_bytes[], char patch_to_bytes[],
              uint64_t patch_from_len, uint64_t patch_to_len)
{
  if (!memmem (patch_from + patch_from_addr, patch_from_len, patch_from_bytes,
               patch_from_len))
    PEXIT (NO_MATCH_FOUND);

  memcpy (patch_to + patch_to_addr, patch_to_bytes, patch_to_len);
  patch_from_idx += patch_from_len;
  patch_to_idx += patch_to_len;
}

void
free_save_file ()
{
  int fd = open (patch_to_name, O_RDWR | O_CREAT);
  write (fd, patch_to->file_buf, patch_to->file_len);
  close (fd);
  free_mmap (patch_to);
  free_mmap (patch_from);
}

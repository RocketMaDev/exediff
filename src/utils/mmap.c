#include "mmap.h"
#include "log.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

mmap_cont *
mmap_file (char *filename)
{
  int fd = open (filename, O_RDONLY);
  if (fd == -1)
    PERROR ("open");

  struct stat st;
  if (fstat (fd, &st) == -1)
    PERROR ("fstat");

  uint64_t filesz = st.st_size;
  mmap_cont *result = malloc (sizeof (mmap_cont));
  result->file_len = filesz;
  result->mem_len = filesz + (filesz % 0x1000) + 0x1000;

  char *file = mmap (NULL, result->mem_len, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE, fd, 0);
  result->content = file;

  if (file == MAP_FAILED)
    PERROR ("mmap");

  if (close (fd) != 0)
    PERROR ("close");

  return result;
}

void
free_mmap (mmap_cont *file)
{
  munmap (file->content, file->mem_len);
  free (file);
}

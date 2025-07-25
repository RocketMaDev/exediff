#include "log.h"
#include "mmap_file.h"
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

mmap_file *
init_mmap_file (char *filename)
{
  int fd = open (filename, O_RDONLY);
  if (fd == -1)
    PERROR ("open");

  struct stat st;
  if (fstat (fd, &st) == -1)
    PERROR ("fstat");

  uint64_t filesz = st.st_size;
  mmap_file *result = malloc (sizeof (mmap_file));
  result->file_len = filesz;
  result->file_buf = mmap (NULL, result->file_len, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE, fd, 0);

  if (result->file_buf == MAP_FAILED)
    PERROR ("mmap");

  if (close (fd) != 0)
    PERROR ("close");

  return result;
}

mmap_file *
init_mmap_anoy (uint64_t len)
{
  mmap_file *result = malloc (sizeof (mmap_file));
  result->file_len = len;
  result->file_buf = mmap (NULL, len, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (result->file_buf == MAP_FAILED)
    PERROR ("mmap");

  return result;
}

void
free_mmap (mmap_file *file)
{
  munmap (file->file_buf, file->file_len);
  free (file);
}

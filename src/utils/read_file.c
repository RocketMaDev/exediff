#include "read_file.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

FILE *fp = NULL;

void
init_fget (char *filename)
{
  fp = fopen (filename, "r");
  if (fp == NULL)
    PERROR ("fopen");
}

uint64_t
fget_line (char **line)
{
  uint64_t len;
  if (getline (line, &len, fp) != -1)
    return len;
  return -1;
}

void
close_fget (char *line)
{
  free (line);
  fclose (fp);
}

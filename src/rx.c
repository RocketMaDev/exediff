#include "rx.h"
#include "log.h"
#include <fcntl.h>
#include <gelf.h>
#include <libelf.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

map_pages
get_rx (char *elf_path)
{
  map_pages result = { NULL, 0 };
  int fd = -1;
  Elf *elf = NULL;
  GElf_Phdr phdr;
  size_t phnum;

  if (elf_version (EV_CURRENT) == EV_NONE)
    PEXIT (ELF_INIT);

  fd = open (elf_path, O_RDONLY);
  if (fd < 0)
    PERROR ("open");

  elf = elf_begin (fd, ELF_C_READ, NULL);
  if (elf == NULL)
    PERROR (ELF_BEGIN);

  if (elf_kind (elf) != ELF_K_ELF)
    PEXIT (NOT_ELF);

  if (elf_getphdrnum (elf, &phnum) != 0)
    PEXIT (ELF_GETPHDRNUM);

  uint32_t rx_count = 0;
  for (size_t i = 0; i < phnum; i++)
    {
      if (gelf_getphdr (elf, i, &phdr) != &phdr)
        PEXIT (GELF_GETPHDR);

      if (phdr.p_type == PT_LOAD && (phdr.p_flags & PF_R)
          && (phdr.p_flags & PF_X))
        rx_count++;
    }

  if (rx_count == 0)
    goto cleanup;

  result.pages = (map_page *)malloc (rx_count * sizeof (map_page));
  if (result.pages == NULL)
    PERROR ("malloc");

  uint32_t index = 0;
  for (size_t i = 0; i < phnum; i++)
    {
      if (gelf_getphdr (elf, i, &phdr) != &phdr)
        PEXIT (GELF_GETPHDR);

      if (phdr.p_type == PT_LOAD && (phdr.p_flags & PF_R)
          && (phdr.p_flags & PF_X))
        {
          result.pages[index].page_adr = phdr.p_vaddr;
          result.pages[index].page_len = (uint32_t)phdr.p_memsz;
          index++;
        }
    }

  result.page_num = rx_count;

cleanup:
  if (elf != NULL)
    elf_end (elf);
  if (fd >= 0)
    close (fd);

  return result;
}

void
free_map_pages (map_pages *pages)
{
  if (pages && pages->pages)
    {
      free (pages->pages);
      pages->pages = NULL;
      pages->page_num = 0;
    }
}

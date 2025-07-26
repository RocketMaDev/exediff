#ifndef RX
#define RX

#include <stdint.h>

typedef struct {
    uint64_t page_adr;
    uint32_t page_len;
} map_page;

typedef struct {
    map_page *pages;
    uint32_t page_num;
} map_pages;

map_pages get_rx(char *elf_path);

void free_map_pages(map_pages *pages);

#endif

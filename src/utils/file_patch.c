#include "file_patch.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "log.h"
#include "mmap_file.h"

mmap_file *patch_to = NULL;
mmap_file *patch_from = NULL;
uint32_t patch_to_idx = 0;
uint32_t patch_from_idx = 0;

char *patch_to_name = NULL;

bool init_patch(uint32_t filesz, char *file_name) {
    patch_to = init_mmap_anoy(filesz + (0x2000 - (filesz % 0x1000)));
    patch_from = init_mmap_file(file_name);

    patch_to_name = file_name;
    return true;
}

void copy_until_hunk(uint32_t copy_until) {
    memcpy(patch_to->file_buf + patch_to_idx, patch_from->file_buf + patch_from_idx,
           copy_until - patch_to_idx);
    patch_from_idx = copy_until;
    patch_to_idx = copy_until;
}

#define SLIDE_WINDOW 3
int32_t slide_match(uint64_t patch_from_addr, char patch_from_bytes[],
                    uint64_t patch_from_len) {
    for (int32_t i = -SLIDE_WINDOW; i < SLIDE_WINDOW; i++)
        if (memmem(patch_from->file_buf + patch_from_addr + i, patch_from_len,
                   patch_from_bytes, patch_from_len))
            return i;
    PEXIT(NO_MATCH_FOUND);
}

void replace_hunk(uint64_t patch_from_addr, uint64_t patch_to_addr,
                  char patch_from_bytes[], char patch_to_bytes[], uint64_t patch_from_len,
                  uint64_t patch_to_len) {
    int32_t offset = slide_match(patch_from_addr, patch_from_bytes, patch_from_len);

    memcpy(patch_to->file_buf + patch_to_addr + offset, patch_to_bytes, patch_to_len);
    patch_from_idx += patch_from_len;
    patch_to_idx += patch_to_len;
}

void free_save_file() {
    copy_until_hunk(patch_from->file_len - patch_from_idx + patch_to_idx);

    remove(patch_to_name);
    int fd = open(patch_to_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    write(fd, patch_to->file_buf, patch_from->file_len - patch_from_idx + patch_to_idx);
    close(fd);
    free_mmap(patch_to);
    free_mmap(patch_from);
}

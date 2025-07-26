#include <fcntl.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

#include "bytes_or_asm.h"
#include "file_patch.h"
#include "hunks.h"
#include "log.h"
#include "read_file.h"
#include "str.h"

void copy_orig_file(char *file_name, uint32_t filesz) {
    uint32_t file_name_len = strlen(file_name);
    char *file_name_orig = malloc(file_name_len + strlen(".orig"));
    if (file_name_orig == NULL)
        PERROR("malloc");

    sprintf(file_name_orig, "%s.orig", file_name);
    int store = open(file_name_orig, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (store == -1)
        PERROR("open");
    free(file_name_orig);

    int orig = open(file_name, O_RDONLY);

    sendfile(store, orig, 0, filesz);
    close(orig);
    close(store);
}

bool get_patch_to(char *file_name, bool *has_file) {
    int fd = open(file_name, O_RDONLY);
    if (fd == -1)
        PERROR("open");

    struct stat st;
    if (fstat(fd, &st) == -1)
        PERROR("fstat");
    close(fd);

    uint64_t filesz = st.st_size;
    copy_orig_file(file_name, filesz);

    if (*has_file == true)
        free_save_file();
    *has_file = init_patch(filesz, file_name);
    return has_file;
}

void resolve_hunks(char *hunk_line) {
    uint32_t patch_from_addr = hunk_patch_from_addr(hunk_line);
    uint32_t patch_from_expect_len = hunk_patch_from_len(hunk_line);
    uint32_t patch_to_addr = hunk_patch_to_addr(hunk_line);
    uint32_t patch_to_expect_len = hunk_patch_to_len(hunk_line);

    copy_until_hunk(patch_from_addr, patch_to_addr);

    char patch_from[patch_from_expect_len * LINE_LEN];
    char patch_to[patch_to_expect_len * LINE_LEN];
    uint32_t patch_from_len = 0;
    uint32_t patch_to_len = 0;

    char *line;
    while (fget_line(&line) != (uint64_t)-1) {
        switch (line[0]) {
            case ' ':
                bytes_or_asm(&line[1], &patch_from_len, patch_from);
                bytes_or_asm(&line[1], &patch_to_len, patch_to);
                break;
            case '-':
                bytes_or_asm(&line[1], &patch_from_len, patch_from);
                break;
            case '+':
                bytes_or_asm(&line[1], &patch_to_len, patch_to);
                break;
            default:
                PEXIT(INVALID_CTX_LEN);
        }

        if (patch_from_len == patch_from_expect_len &&
            patch_to_len == patch_to_expect_len)
            break;
    }

    replace_hunk(patch_from_addr, patch_to_addr, patch_from, patch_to, patch_from_len,
                 patch_to_len);
}

int main(int argc, char *argv[]) {
    init_fget(argv[1]);

    char *line = NULL;
    bool has_file = false;

    while (fget_line(&line) != (uint64_t)-1)
        if (start_with(line, "--- "))
            get_patch_to(line + strlen("--- "), &has_file);
        else if (!has_file)
            PEXIT(NO_TARGET_FILE)

        else if (start_with(line, "+++ "))
            continue;

        else if (start_with(line, "@@ -"))
            resolve_hunks(line + strlen("@@ -"));

    free_save_file();
    free_fget(line);
}

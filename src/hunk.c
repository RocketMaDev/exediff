#include "hunk.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "minmax.h"
#include "mmap_file.h"

list *deleted;
list *inserted;

typedef struct {
    // offset cursors
    unsigned start;
    unsigned cursor;
    unsigned end;
    unsigned size;
    // hunk bottom & top
    long bot;
    long top;
    // array in list
    long *arr;
    // file attributes
    uint8_t *file_buf;
    long file_len;
} hunk_diff;

static void print_hunk_header(long old_pos, long old_len, long new_pos, long new_len) {
    printf("@@ -%#lx,%ld +%#lx,%ld @@\n", old_pos, old_len, new_pos, new_len);
}

#define HEXSTR "0123456789abcdef"
static void print_hex_line(uint8_t *data, unsigned size, char prepend) {
    if (!size)
        return;
    putchar(prepend);
    unsigned i = 0;
    while (i < size - 1) {
        putchar(HEXSTR[data[i] >> 4]);
        putchar(HEXSTR[data[i] & 0xf]);
        if ((i++ & 0xf) == 0xf) {
            putchar('\n');
            putchar(prepend);
            continue;
        }
        putchar(' ');
    }
    putchar(HEXSTR[data[i] >> 4]);
    putchar(HEXSTR[data[i] & 0xf]);
    putchar('\n');
}

static long min(long a, long b) {
    return MIN(a, b);
}

static long max(long a, long b) {
    return MAX(a, b);
}
#define CTX 3

// Test the length of longest continuous interger string in array.
static inline long continuous_len(unsigned size, unsigned idx, long *array) {
    if (idx >= size)
        return 0;
    register long *ptr = array + idx + 1;
    register long *end = array + size;
    while (ptr < end && *ptr - *(ptr - 1) == 1) ptr++;
    return ptr - (array + idx);
}

static void handle_one_side(hunk_diff *blk_avail, hunk_diff *blk_over, bool is_new_over) {
    char prepend = is_new_over ? '-' : '+';
    hunk_diff blk = *blk_avail;
    while (blk.cursor < blk.size) {
        blk.start = blk.cursor;
        long diff_point = blk.arr[blk.cursor];
        blk.bot = max(0, diff_point - CTX);
        long diff = blk.bot - blk.top;
        long length = continuous_len(blk.size, blk.cursor, blk.arr);
        blk.cursor += length;
        blk.top = min(blk.bot + 2 * CTX + 1 + length, blk.file_len);
        blk_over->bot = blk_over->top + diff;
        blk_over->top = min(blk_over->top + 2 * CTX + 1, blk_over->file_len);
        if (is_new_over)
            print_hunk_header(blk.bot, blk.top - blk.bot, blk_over->bot,
                              blk_over->top - blk_over->bot);
        else
            print_hunk_header(blk_over->bot, blk_over->top - blk_over->bot, blk.bot,
                              blk.top - blk.bot);
        print_hex_line(blk.file_buf + blk.bot, blk.arr[blk.start] - blk.bot, ' ');
        print_hex_line(blk.file_buf + diff_point, length, prepend);
        diff_point += length;
        print_hex_line(blk.file_buf + diff_point, blk.top - diff_point, ' ');
    }
}

// print context here
static void handle_normal(hunk_diff *_old, hunk_diff *_new) {
    hunk_diff old = *_old;
    hunk_diff new = *_new;
    print_hunk_header(old.bot, old.top - old.bot, new.bot, new.top - new.bot);
    long old_reader = old.bot, new_reader = new.bot;
    long old_part = old_reader, new_part = new_reader;
    old.end = min(old.end, old.size);
    new.end = min(new.end, new.size);
    old.cursor = old.start, new.cursor = new.start;
    while (old_reader < old.top && new_reader < new.top) {
        while ((old.cursor >= old.end || old_reader != old.arr[old.cursor]) &&
               (new.cursor >= new.end || new_reader != new.arr[new.cursor]) &&
               old_reader < old.top && new_reader < new.top)
            // the byte in old and new is the same
            old_reader++, new_reader++;
        // now the SHARED part is ready
        print_hex_line(old.file_buf + old_part, old_reader - old_part, ' ');
        if (old.cursor < old.end && old_reader == old.arr[old.cursor]) {
            long length = continuous_len(old.end, old.cursor, old.arr);
            print_hex_line(old.file_buf + old_reader, length, '-');
            old.cursor += length;
            old_reader += length;
            old_part = old_reader;
        }
        if (new.cursor < new.end && new_reader == new.arr[new.cursor]) {
            long length = continuous_len(new.end, new.cursor, new.arr);
            print_hex_line(new.file_buf + new_reader, length, '+');
            new.cursor += length;
            new_reader += length;
            old_part = old_reader;
        }
    }
    // printf("%d %d %d %d %p %p %p %p\n", old.cursor, old.end, new.cursor, new.end,
    // old_reader, old.top, new_reader, new.top);
    assert(old.cursor == old.end);
    assert(new.cursor == new.end);
    assert(new_reader == new.top);
    assert(old_reader == old.top);
}

void handle_delta(mmap_file *old_file, mmap_file *new_file) {
    // clang-format off
    hunk_diff old = { .cursor = 0, .start = 0, .end = 0, .size = deleted->size,
        .arr = deleted->array, .file_buf = old_file->file_buf, .file_len = old_file->file_len };
    hunk_diff new = { .cursor = 0, .start = 0, .end = 0, .size = inserted->size,
        .arr = inserted->array, .file_buf = new_file->file_buf, .file_len = new_file->file_len };
    // clang-format on
    long file_len_min = min(old.file_len, new.file_len);
    long diff = 0;
    if (!old.size && !new.size)
        return;
    else if (!old.size) {
        handle_one_side(&new, &old, false);
        return;
    } else if (!new.size) {
        handle_one_side(&old, &new, true);
        return;
    }
    // if deleted size or inserted size is 0, then arr[1] == 0 is implied
    if (old.arr[old.cursor] <= new.arr[new.cursor]) {
        if (old.arr[old.cursor] == new.arr[new.cursor])
            new.end = ++new.cursor;
        old.bot = new.bot = max(0, old.arr[old.cursor++] - CTX);
        old.end = old.cursor;
    } else {
        new.bot = old.bot = max(0, new.arr[new.cursor++] - CTX);
        new.end = new.cursor;
    }
    old.top = new.top = min(file_len_min, old.bot + 2 * CTX + 1);
    while (old.cursor < old.size || new.cursor < new.size) {
        bool enlarged = false;
        if (old.cursor < old.size && old.arr[old.cursor] < old.top + 2 * CTX + 1) {
            old.top = min(old.file_len, max(old.top, old.arr[old.cursor++] + CTX + 1));
            old.end = old.cursor;  // exclude
            enlarged = true;
        }
        if (new.cursor < new.size && new.arr[new.cursor] < new.top + 2 * CTX + 1) {
            new.top = min(new.file_len, max(new.top, new.arr[new.cursor++] + CTX + 1));
            new.end = new.cursor;
            enlarged = true;
        }
        if (!enlarged) {
            // adjust top
            long shared_size = max(old.top - old.bot - old.end + old.start,
                                   new.top - new.bot - new.end + new.start);
            old.top = old.bot + shared_size + old.end - old.start;
            new.top = new.bot + shared_size + new.end - new.start;
            if (old.top > old.file_len || new.top > new.file_len) {
                old.top = min(old.top, old.file_len);
                new.top = min(new.top, new.file_len);
                shared_size = max(old.top - old.bot - old.end + old.start,
                                  new.top - new.bot - new.end + new.start);
                old.top = old.bot + shared_size + old.end - old.start;
                new.top = new.bot + shared_size + new.end - new.start;
            }
            handle_normal(&old, &new);
            old.start = old.end, new.start = new.end;
            assert(old.cursor < old.size || new.cursor < new.size);
            if (old.cursor >= old.size) {
                handle_one_side(&new, &old, false);
                return;
            } else if (new.cursor >= new.size) {
                handle_one_side(&old, &new, true);
                return;
                // not absolute offset, but relative offset! $ - top is needed
            } else if (old.arr[old.cursor] - old.top <= new.arr[new.cursor] - new.top) {
                old.bot = old.arr[old.cursor++] - CTX;
                diff = old.bot - old.top;
                old.end = old.cursor;
                old.top = min(old.file_len, old.bot + 2 * CTX + 1);
                new.bot = min(new.file_len, new.top + diff);
                new.top = min(new.file_len, new.bot + 2 * CTX + 1);
            } else if (old.arr[old.cursor] - old.top > new.arr[new.cursor] - new.top) {
                new.bot = new.arr[new.cursor++] - CTX;
                diff = new.bot - new.top;
                new.end = new.cursor;
                new.top = min(new.file_len, new.bot + 2 * CTX + 1);
                old.bot = min(old.file_len, old.top + diff);
                old.top = min(old.file_len, old.bot + 2 * CTX + 1);
            }
        }
    }
    if (old.cursor >= old.size && new.cursor >= new.size) {
        long shared_size = max(old.top - old.bot - old.end + old.start,
                               new.top - new.bot - new.end + new.start);
        old.top = old.bot + shared_size + old.end - old.start;
        new.top = new.bot + shared_size + new.end - new.start;
        if (old.top > old.file_len || new.top > new.file_len) {
            old.top = min(old.top, old.file_len);
            new.top = min(new.top, new.file_len);
            shared_size = max(old.top - old.bot - old.end + old.start,
                              new.top - new.bot - new.end + new.start);
            old.top = old.bot + shared_size + old.end - old.start;
            new.top = new.bot + shared_size + new.end - new.start;
        }
        handle_normal(&old, &new);
    }
}

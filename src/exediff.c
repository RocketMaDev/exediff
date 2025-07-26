/* Optimized binary file difference analyzer with snake optimization
   Created by RocketMaDev on 2025-07-24

   This version includes enhanced snake detection and skipping mechanisms.
*/

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "hunk.h"
#include "list.h"

/* Configuration for snake optimization */
#define SNAKE_LIMIT 20           /* Minimum snake length to be considered "big" */
#define FAST_SKIP_THRESHOLD 1024 /* Use fast skipping for blocks larger than this */

/* Forward declarations */
struct file_data {
    uint8_t *data;
    size_t size;
    const char *name;
};

struct diff_context {
    struct file_data files[2];
    size_t change_count;
    size_t snake_count;        /* Number of snakes found */
    size_t total_snake_length; /* Total length of all snakes */
    bool verbose;
};

static struct diff_context current_diff;

/* Enhanced macros for diffseq.h with snake tracking */
#define ELEMENT uint8_t
#define EQUAL(x, y) ((x) == (y))
#define OFFSET ptrdiff_t
#define OFFSET_MAX PTRDIFF_MAX

#define EXTRA_CONTEXT_FIELDS       \
    struct diff_context *diff_ctx; \
    size_t current_snake_length;

#define NOTE_DELETE(ctxt, xoff) note_byte_delete((xoff))

#define NOTE_INSERT(ctxt, yoff) note_byte_insert((yoff))

#define USE_HEURISTIC
static void note_byte_insert(ptrdiff_t offset);
static void note_byte_delete(ptrdiff_t offset);

#include "../lib/diffseq.h"
#include "../lib/minmax.h"

/* Enhanced snake detection function */
static size_t find_snake_fast(const uint8_t *data1, size_t pos1, size_t limit1,
                              const uint8_t *data2, size_t pos2, size_t limit2) {
    size_t snake_length = 0;
    size_t remaining1 = limit1 - pos1;
    size_t remaining2 = limit2 - pos2;
    size_t max_compare = MIN(remaining1, remaining2);

    if (max_compare == 0)
        return 0;

    /* Fast block comparison for large identical sections */
    if (max_compare >= FAST_SKIP_THRESHOLD) {
        size_t block_size = FAST_SKIP_THRESHOLD;

        while (snake_length + block_size <= max_compare) {
            if (memcmp(data1 + pos1 + snake_length, data2 + pos2 + snake_length,
                       block_size) == 0) {
                snake_length += block_size;
            } else {
                break;
            }
        }
    }

    /* Byte-by-byte comparison for remaining data */
    while (snake_length < max_compare &&
           data1[pos1 + snake_length] == data2[pos2 + snake_length]) {
        snake_length++;
    }

    return snake_length;
}

/* Enhanced snake detection in reverse direction */
static size_t find_snake_reverse_fast(const uint8_t *data1, size_t pos1, size_t start1,
                                      const uint8_t *data2, size_t pos2, size_t start2) {
    size_t snake_length = 0;
    size_t remaining1 = pos1 - start1;
    size_t remaining2 = pos2 - start2;
    size_t max_compare = MIN(remaining1, remaining2);

    if (max_compare == 0)
        return 0;

    /* Fast block comparison for large identical sections */
    if (max_compare >= FAST_SKIP_THRESHOLD) {
        size_t block_size = FAST_SKIP_THRESHOLD;

        while (snake_length + block_size <= max_compare) {
            if (memcmp(data1 + pos1 - snake_length - block_size,
                       data2 + pos2 - snake_length - block_size, block_size) == 0) {
                snake_length += block_size;
            } else {
                break;
            }
        }
    }

    /* Byte-by-byte comparison for remaining data */
    while (snake_length < max_compare &&
           data1[pos1 - snake_length - 1] == data2[pos2 - snake_length - 1]) {
        snake_length++;
    }

    return snake_length;
}

/* Custom compareseq wrapper with enhanced snake detection */
static bool enhanced_compareseq(ptrdiff_t xoff, ptrdiff_t xlim, ptrdiff_t yoff,
                                ptrdiff_t ylim, bool find_minimal, struct context *ctxt) {
    const uint8_t *data1 = current_diff.files[0].data;
    const uint8_t *data2 = current_diff.files[1].data;

    /* Enhanced initial snake detection */
    size_t front_snake = find_snake_fast(data1, xoff, xlim, data2, yoff, ylim);
    if (front_snake > 0) {
        current_diff.snake_count++;
        current_diff.total_snake_length += front_snake;

        xoff += front_snake;
        yoff += front_snake;
    }

    /* Enhanced trailing snake detection */
    size_t back_snake = find_snake_reverse_fast(data1, xlim, xoff, data2, ylim, yoff);
    if (back_snake > 0) {
        current_diff.snake_count++;
        current_diff.total_snake_length += back_snake;

        xlim -= back_snake;
        ylim -= back_snake;
    }

    /* If nothing left to compare after snake removal */
    if (xoff >= xlim && yoff >= ylim)
        return false; /* No differences in this section */

    /* Handle simple cases after snake removal */
    if (xoff >= xlim) {
        /* Only insertions remain */
        for (ptrdiff_t i = yoff; i < ylim; i++) NOTE_INSERT(ctxt, i);
        return false;
    }

    if (yoff >= ylim) {
        /* Only deletions remain */
        for (ptrdiff_t i = xoff; i < xlim; i++) NOTE_DELETE(ctxt, i);
        return false;
    }

    /* Use the original Myers algorithm for the remaining differences */
    return compareseq(xoff, xlim, yoff, ylim, find_minimal, ctxt);
}

/* Callback functions */
static void note_byte_delete(ptrdiff_t offset) {
    current_diff.change_count++;
    list_append(deleted, offset);
}

static void note_byte_insert(ptrdiff_t offset) {
    current_diff.change_count++;
    list_append(inserted, offset);
}

/* Utility functions for file operations */
static int read_file(const char *filename, struct file_data *file) {
    FILE *fp;
    struct stat st;

    file->name = filename;

    if (stat(filename, &st) != 0) {
        fprintf(stderr, "Error: Cannot stat file '%s': %s\n", filename, strerror(errno));
        return -1;
    }

    file->size = st.st_size;

    if (file->size == 0) {
        file->data = NULL;
        return 0;
    }

    file->data = malloc(file->size);
    if (!file->data) {
        fprintf(stderr, "Error: Cannot allocate %zu bytes for file '%s'\n", file->size,
                filename);
        return -1;
    }

    fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file '%s': %s\n", filename, strerror(errno));
        free(file->data);
        file->data = NULL;
        return -1;
    }

    size_t bytes_read = fread(file->data, 1, file->size, fp);
    fclose(fp);

    if (bytes_read != file->size) {
        fprintf(stderr, "Error: Read only %zu of %zu bytes from '%s'\n", bytes_read,
                file->size, filename);
        free(file->data);
        file->data = NULL;
        return -1;
    }

    return 0;
}

static void free_file(struct file_data *file) {
    if (file->data) {
        free(file->data);
        file->data = NULL;
    }
    file->size = 0;
}

/* Quick similarity check using sampling */
static double quick_similarity_check(const struct file_data *file1,
                                     const struct file_data *file2) {
    if (file1->size == 0 && file2->size == 0)
        return 1.0; /* Both empty */

    if (file1->size == 0 || file2->size == 0)
        return 0.0; /* One empty, one not */

    /* Sample every 1024th byte for quick similarity estimate */
    size_t sample_interval = 1024;
    size_t samples = 0;
    size_t matches = 0;

    size_t max_samples = MIN(file1->size, file2->size) / sample_interval;
    if (max_samples == 0) {
        max_samples = MIN(file1->size, file2->size);
        sample_interval = 1;
    }

    for (size_t i = 0; i < max_samples * sample_interval; i += sample_interval) {
        if (i < file1->size && i < file2->size) {
            samples++;
            if (file1->data[i] == file2->data[i])
                matches++;
        }
    }

    return samples > 0 ? (double)matches / samples : 0.0;
}

/* Main comparison function with enhanced optimization */
static int compare_files_optimized(const char *file1, const char *file2, bool verbose,
                                   bool minimal) {
    struct context ctx;
    int result = 0;

    /* Initialize global context */
    memset(&current_diff, 0, sizeof(current_diff));
    current_diff.verbose = verbose;

    /* Read both files */
    if (read_file(file1, &current_diff.files[0]) != 0)
        return -1;

    if (read_file(file2, &current_diff.files[1]) != 0) {
        free_file(&current_diff.files[0]);
        return -1;
    }

    /* Quick similarity check */

    /* Handle identical files quickly */
    if (current_diff.files[0].size == current_diff.files[1].size) {
        if (current_diff.files[0].size == 0)
            goto cleanup;

        if (memcmp(current_diff.files[0].data, current_diff.files[1].data,
                   current_diff.files[0].size) == 0) {
            goto cleanup;
        }
    }

    /* Setup context for Myers algorithm */
    memset(&ctx, 0, sizeof(ctx));
    ctx.xvec = current_diff.files[0].data;
    ctx.yvec = current_diff.files[1].data;
    ctx.diff_ctx = &current_diff;

    /* Calculate diagonal array size */
    ptrdiff_t diags =
        (ptrdiff_t)(current_diff.files[0].size + current_diff.files[1].size + 3);

    /* Allocate diagonal arrays */
    ctx.fdiag = malloc(diags * 2 * sizeof(ptrdiff_t));
    if (!ctx.fdiag) {
        fprintf(stderr, "Error: Cannot allocate diagonal arrays\n");
        result = -1;
        goto cleanup;
    }

    ctx.bdiag = ctx.fdiag + diags;
    ctx.fdiag += current_diff.files[1].size + 1;
    ctx.bdiag += current_diff.files[1].size + 1;

    /* Set algorithm parameters */
    ctx.heuristic = !minimal;
    // ctx.too_expensive = MAX(4096, (ptrdiff_t)(similarity < 0.8 ?
    //                        diags / 10 : diags / 4));
    ctx.too_expensive = 0x1000000000;

    /* Run the enhanced comparison algorithm */

    bool aborted =
        enhanced_compareseq(0, (ptrdiff_t)current_diff.files[0].size, 0,
                            (ptrdiff_t)current_diff.files[1].size, minimal, &ctx);

    /* Report detailed results */

    /* Free diagonal arrays */
    free(ctx.fdiag - (current_diff.files[1].size + 1));

cleanup:
    // free_file(&current_diff.files[0]);
    // free_file(&current_diff.files[1]);

    return result;
}

/* Main function */
int main(int argc, char *argv[]) {
    bool verbose = false;
    bool minimal = false;
    const char *file1 = NULL;
    const char *file2 = NULL;
    deleted = list_new();
    inserted = list_new();

    /* Parse command line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--minimal") == 0) {
            minimal = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            return 0;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            return 2;
        } else if (file1 == NULL) {
            file1 = argv[i];
        } else if (file2 == NULL) {
            file2 = argv[i];
        } else {
            fprintf(stderr, "Error: Too many file arguments\n");
            return 2;
        }
    }

    /* Validate arguments */
    if (file1 == NULL || file2 == NULL) {
        fprintf(stderr, "Error: Two file arguments required\n");
        return 2;
    }

    /* Check if files exist */
    if (access(file1, R_OK) != 0) {
        fprintf(stderr, "Error: Cannot read file '%s': %s\n", file1, strerror(errno));
        return 2;
    }

    if (access(file2, R_OK) != 0) {
        fprintf(stderr, "Error: Cannot read file '%s': %s\n", file2, strerror(errno));
        return 2;
    }

    /* Perform the optimized comparison */
    int result = compare_files_optimized(file1, file2, verbose, minimal);
    list_sort(deleted);
    list_sort(inserted);

    // for (unsigned i = 0; i < deleted->size; i++)
    //     printf("DELETED: %u, %ld [%02x]\tINSERTED: %u, %ld [%02x]\n",
    //            i, deleted->array[i], current_diff.files[0].data[deleted->array[i]],
    //            i, inserted->array[i], current_diff.files[1].data[inserted->array[i]]);

    mmap_file old_file = {current_diff.files[0].data, current_diff.files[0].size};
    mmap_file new_file = {current_diff.files[1].data, current_diff.files[1].size};
    handle_delta(&old_file, &new_file);

    if (result < 0)
        return 2;

    return result;
}

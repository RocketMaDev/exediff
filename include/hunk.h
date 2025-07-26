#ifndef HUNK_H
#define HUNK_H
#include <stdint.h>

#include "list.h"
#include "mmap_file.h"
extern list *deleted;
extern list *inserted;

void handle_delta(mmap_file *old_file, mmap_file *new_file);
#endif

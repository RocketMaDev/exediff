#include "hunk.h"
#include "../lib/minmax.h"
#include "list.h"
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

list *deleted;
list *inserted;

typedef struct
{
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

static void
print_hunk_header (long old_pos, long old_len, long new_pos, long new_len)
{
  printf ("@@ -%#lx,%ld +%#lx,%ld @@\n", old_pos, old_len, new_pos, new_len);
}

#define HEXSTR "0123456789abcdef"
static void
print_hex_line (uint8_t *data, unsigned size, char prepend)
{
  if (!size)
    return;
  putchar (prepend);
  unsigned i = 0;
  while (i < size - 1)
    {
      putchar (HEXSTR[data[i] >> 4]);
      putchar (HEXSTR[data[i] & 0xf]);
      if ((i++ & 0xf) == 0xf)
        {
          putchar ('\n');
          putchar (prepend);
          continue;
        }
      putchar (' ');
    }
  putchar (HEXSTR[data[i] >> 4]);
  putchar (HEXSTR[data[i] & 0xf]);
  putchar ('\n');
}

static long
min (long a, long b)
{
  return MIN (a, b);
}

static long
max (long a, long b)
{
  return MAX (a, b);
}
#define CTX 3

static void
handle_edges (hunk_diff *old, hunk_diff *new)
{
  if (old->cursor >= old->size)
    {
      assert (new->cursor < new->size);
      while (new->arr[new->cursor] < new->file_len &&new->cursor < new->size)
        {
        }
      if (new->cursor == new->size)
        return;
      // print rest content in new
      unsigned extra_pos_idx = new->cursor;
      long *array = new->arr;
      long extra_pos = array[extra_pos_idx];
      long more = extra_pos - max (0, extra_pos - CTX);
      long rolling = extra_pos;

      // assert rest changes to be sequential
      for (unsigned i = extra_pos_idx + 1; i < new->size; i++)
        if (++rolling != array[i])
          {
            puts ("Inconsistent diff in new file end?");
            exit (3);
          }

      // checked successfully
      print_hunk_header (old->file_len, 0, extra_pos - more,
                         new->size - new->cursor + more);
      print_hex_line (new->file_buf + extra_pos - more,
                      new->size - new->cursor + more, '+');
      return;
    }
  assert (new->cursor >= new->size);
  while (old->arr[old->cursor] < new->file_len && old->cursor < old->size)
    {
    }
  if (old->cursor == old->size)
    return;
  unsigned extra_pos_idx = old->cursor;
  long *array = old->arr;
  long extra_pos = array[extra_pos_idx];
  long more = extra_pos - max (0, extra_pos - CTX);
  long rolling = extra_pos;

  // assert rest changes to be sequential
  for (unsigned i = extra_pos_idx + 1; i < old->size; i++)
    if (++rolling != array[i])
      {
        puts ("Inconsistent diff in old file end?");
        exit (3);
      }

  // checked successfully
  print_hunk_header (extra_pos - more, old->size - old->cursor + more,
                     new->file_len, 0);
  print_hex_line (old->file_buf + extra_pos - more,
                  old->size - old->cursor + more, '-');
}

// print context here
static void
handle_normal (hunk_diff *_old, hunk_diff *_new)
{
  hunk_diff old = *_old;
  hunk_diff new = *_new;
  print_hunk_header (old.bot, old.top - old.bot, new.bot, new.top - new.bot);
  long old_reader = old.bot, new_reader = new.bot;
  long old_part = old_reader, new_part = new_reader;
  old.cursor = old.start, new.cursor = new.start;
  while (old_reader < old.top || new_reader < new.top)
    {
      while (old_reader != old.arr[old.cursor]
             && new_reader != new.arr[new.cursor]
             && old_reader < old.top &&new_reader < new.top)
        // the byte in old and new is the same
        old_reader++, new_reader++;
      // now the SHARED part is ready
      print_hex_line (old.file_buf + old_part, old_reader - old_part, ' ');
      if (old.cursor < old.end && old_reader == old.arr[old.cursor])
        {
          long rolling = old.arr[old.cursor++] + 1;
          while (old.cursor < old.end && old.arr[old.cursor++] == rolling++)
            ;
          // now rolling is next byte not deleted
          print_hex_line (old.file_buf + old_reader, rolling - old_reader,
                          '-');
          old_part = old_reader = rolling;
        }
      if (new.cursor < new.end &&new_reader == new.arr[new.cursor])
        {
          long rolling = new.arr[new.cursor++] + 1;
          while (new.cursor < new.end &&new.arr[new.cursor++] == rolling++)
            ;
          // now rolling is next byte not deleted
          print_hex_line (new.file_buf + new_reader, rolling - new_reader,
                          '+');
          new_part = new_reader = rolling;
        }
    }
  // printf("%d %d %d %d %p %p %p %p\n", old.cursor, old.end, new.cursor,
  // new.end, old_reader, old.top, new_reader, new.top);
  assert (old.cursor == old.end);
  assert (new.cursor == new.end);
  assert (old_reader == old.top);
  assert (new_reader == new.top);
}

void
handle_delta (mmap_file *old_file, mmap_file *new_file)
{
  hunk_diff old = { .cursor = 0,
                    .start = 0,
                    .end = 0,
                    .size = deleted->size,
                    .arr = deleted->array,
                    .file_buf = old_file->file_buf,
                    .file_len = old_file->file_len };
  hunk_diff new = { .cursor = 0,
                    .start = 0,
                    .end = 0,
                    .size = inserted->size,
                    .arr = inserted->array,
                    .file_buf = new_file->file_buf,
                    .file_len = new_file->file_len };
  long file_len_min = MIN (old.file_len, new.file_len);
  long diff = 0;
  if (!old.size && !new.size)
    return;
  // if deleted size or inserted size is 0, then arr[1] == 0 is implied
  if (old.arr[old.cursor] <= new.arr[new.cursor])
    {
      if (old.arr[old.cursor] == new.arr[new.cursor])
        new.end = ++new.cursor;
      old.bot = new.bot = max (0, old.arr[old.cursor++] - CTX);
      old.end = old.cursor;
    }
  else
    {
      new.bot = old.bot = max (0, new.arr[new.cursor++] - CTX);
      new.end = new.cursor;
    }
  old.top = new.top = min (file_len_min, old.bot + 2 * CTX + 1);
  while (old.cursor < old.size || new.cursor < new.size)
    {
      bool enlarged = false;
      if (old.cursor < old.size && old.arr[old.cursor] < old.top + 2 * CTX + 1)
        {
          old.top = min (old.file_len,
                         max (old.top, old.arr[old.cursor++] + CTX + 1));
          old.end = old.cursor; // exclude
          enlarged = true;
        }
      if (new.cursor < new.size &&new.arr[new.cursor] < new.top + 2 * CTX + 1)
        {
          new.top = min (new.file_len,
                         max (new.top, new.arr[new.cursor++] + CTX + 1));
          new.end = new.cursor;
          enlarged = true;
        }
      if (!enlarged)
        {
          handle_normal (&old, &new);
          old.start = old.end, new.start = new.end;
          if (old.arr[old.cursor] < new.arr[new.cursor])
            {
              old.bot = old.arr[old.cursor++] - CTX;
              diff = old.bot - old.top;
              old.end = old.cursor;
              old.top = min (old.file_len, old.bot + 2 * CTX + 1);
              new.bot = min (new.file_len, new.top + diff);
              new.top = min (new.file_len, new.bot + 2 * CTX + 1);
            }
          else if (old.arr[old.cursor] > new.arr[new.cursor])
            {
              new.bot = new.arr[new.cursor++] - CTX;
              diff = new.bot - new.top;
              new.end = new.cursor;
              new.top = min (new.file_len, new.bot + 2 * CTX + 1);
              old.bot = min (old.file_len, old.top + diff);
              old.top = min (old.file_len, old.bot + 2 * CTX + 1);
            }
          else
            {
              assert (old.cursor == new.cursor);
              old.bot = new.bot = old.arr[old.cursor++] - CTX;
              new.cursor++;
              old.end = old.cursor, new.end = new.cursor;
              old.top = min (old.file_len, old.bot + 2 * CTX + 1);
              new.top = min (new.file_len, old.bot + 2 * CTX + 1);
            }
        }
    }
  if (old.cursor == old.size && new.cursor == new.size)
    {
      handle_normal (&old, &new);
    }
}

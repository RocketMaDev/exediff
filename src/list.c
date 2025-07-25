#include "list.h"
#include <stdlib.h>
#include <string.h>

static int
grow (list *list, int size)
{
  if (size <= 0)
    {
      // auto expand
      size = list->capacity + (list->capacity >> 1);
      if (size < MIN_SIZE)
        size = MIN_SIZE;
    }
  else
    size = size <= 11 ? MIN_SIZE : size;
  NODE_TYPE *array
      = (NODE_TYPE *)realloc (list->array, sizeof (NODE_TYPE) * size);
  // IFOOM(array);
  list->array = array;
  list->capacity = size;
  // return -RERR_OK;
  return 0;
}

list *
list_new (void)
{
  list *newlist = (list *)malloc (sizeof (list));
  if (!newlist)
    return NULL;
  newlist->array = NULL;
  newlist->capacity = 0;
  newlist->size = 0;
  grow (newlist, 0);
  memset (newlist->array, 0, sizeof (NODE_TYPE) * newlist->capacity);
  return newlist;
}

void
list_release (list *l)
{
  l->capacity = 0;
  l->size = 0;
  free (l->array);
  l->array = NULL;
  free (l);
}

// trivial comparison
static int
cmp (const void *a, const void *b)
{
  if (*(NODE_TYPE *)a > *(NODE_TYPE *)b)
    return 1;
  else if (*(NODE_TYPE *)a < *(NODE_TYPE *)b)
    return -1;
  else
    return 0;
}

void
list_sort (list *l)
{
  if (!l->size)
    return;
  qsort (l->array, l->size, sizeof (NODE_TYPE), cmp);
}

void
list_append (list *l, NODE_TYPE val)
{
  if (l->size == l->capacity)
    {
      grow (l, 0);
    }
  l->array[l->size++] = val;
}

#ifndef LIST_H
#define LIST_H
#define NODE_TYPE long
typedef struct {
    unsigned size;
    unsigned capacity;
    NODE_TYPE *array;
} list;
#define MIN_SIZE 16

list *list_new();
void list_release(list *l);
void list_sort(list *l);
void list_append(list *l, NODE_TYPE val);
#endif // LIST_H

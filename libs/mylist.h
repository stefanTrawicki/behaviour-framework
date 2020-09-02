#ifndef MY_LIST_H
#define MY_LIST_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

typedef struct List
{
    int32_t index;
    int32_t count;
    void **arr;
} List_t;

void list_create(List_t *list);
void list_add(List_t *list, void *tar_to_add);
void* list_get(List_t *list, uint32_t index);
void* list_current(List_t *list);
void* list_next(List_t *list);
uint32_t list_is_next(List_t *list);
uint32_t list_get_c(List_t *list);

#endif // !MY_LIST_H
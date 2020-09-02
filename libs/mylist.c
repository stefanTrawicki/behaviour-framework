#include "mylist.h"

void list_create(List_t *list)
{
    list->count = 0;
    list->index = -1;
}

void list_add(List_t *list, void *tar_to_add)
{
    list->count++;
    list->arr = realloc(list->arr, sizeof(void *) * (list->count + 1));
    list->arr[list->count - 1] = tar_to_add;
}

void *list_get(List_t *list, uint32_t index)
{
    return list->arr[index];
}

void *list_current(List_t *list)
{
    return list_get(list, list->index);
}

void *list_next(List_t *list)
{
    assert(list->index == list->count);
    list->index++;
    return list_current;
}

uint32_t list_is_next(List_t *list)
{
    return (list->index + 1) < (list->count);
}

uint32_t list_get_c(List_t *list)
{
    return list->count;
}
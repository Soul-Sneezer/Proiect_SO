#include "dynamic_list.h"
#include <stdbool.h>
#include <stdlib.h>

void initList(List* list)
{
    list->count = 0;
    list->size = 0;
    list->elements = NULL;
}

static bool findInList(List* list, uint32_t elem)
{
    for (uint32_t i = 0; i < list->count; i++) {
        if(list->elements[i] == elem)
            return true;
    }

    return false;
}

static void adjustSize(List* list, uint32_t capacity)
{
    uint32_t* elements = (uint32_t*)malloc(capacity * sizeof(uint32_t));

    for (uint32_t i = 0; i < list->count; i++) {
        elements[i] = list->elements[i];
    }

    free(list->elements);
    list->size = capacity;
    list->elements = elements;
}

void addElemToList(List* list, uint32_t elem)
{
    if (list->count + 1 > list->size) {
        uint32_t capacity = GROW_LIST(list->size);
        adjustSize(list, capacity);
    }

    list->elements[list->count++] = elem;
}

void remElemFromList(List* list, uint32_t elem)
{
    for (uint32_t i = 0; i < list->count; i++) {
        if (list->elements[i] == elem) {
            list->elements[i] = list->elements[list->count - 1];
            list->count--;
            return;
        }
    }
}

void freeList(List* list)
{
    free(list->elements);
    initList(list);
}


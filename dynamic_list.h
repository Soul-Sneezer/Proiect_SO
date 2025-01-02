#ifndef TELEMETRY_DYN_LIST
#define TELEMETRY_DYN_LIST

#include <stdlib.h>
#include <stdint.h>

// will replace this with a hash table if I actually implement the feature I'm thinking of

#define GROW_LIST(size) ((size) == 0 ? 8 : 2 * (size))

typedef struct 
{
    uint32_t count;
    uint32_t size;
    uint32_t* elements;
} List;

void initList(List* list);
void addElemToList(List* list, uint32_t elem);
void remElemFromList(List* list, uint32_t elem);
void freeList(List* list);

#endif

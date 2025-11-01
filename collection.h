#pragma once

#include "stdbool.h"
#include <stddef.h>
#include <stdint.h>

typedef int32_t element_id;

struct element_info {
    element_id id;
    void* ptr;
};

typedef void(*foreach_handler)(void* element);
typedef void(*map_handler)(void* element, void* mapped_ptr);

typedef size_t(*length_handler)(void* target);
typedef bool (*has_next_handler)(void* target, element_id* id);

typedef struct element_info (*first_handler)(void* target);
typedef struct element_info (*next_handler)(void* target, element_id* id);

struct collection {
    length_handler length;

    first_handler first;
    next_handler next;
    has_next_handler has_next;
};

struct mapped_collection {
    size_t mapped_type_size;
    map_handler mapper;
    void* target;

    struct collection* collection;
};

void collection_for_each(struct collection* collection, void* target, foreach_handler handler);

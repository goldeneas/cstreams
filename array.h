#pragma once

#include "collection.h"
#include <stdbool.h>
#include <stddef.h>

struct array {
    struct collection collection;

    void* arr;
    size_t element_size;
    size_t capacity;
    size_t length;
};

struct array array_init(size_t elem_size, size_t capacity);
void array_destroy(struct array* array);

void* array_add(void* elem, struct array* arr);
void* array_get(size_t idx, struct array* arr);

struct collection array_collection(void);
struct element_info array_first(void* c);
struct element_info array_next(void* array, element_id id);
bool array_has_next(void* c, element_id id);

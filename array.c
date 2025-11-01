#include "array.h"
#include "collection.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t array_length(void* target) {
    struct array* arr = (struct array*) target;
    return arr->length;
}

bool array_has_next(void* target, element_id id) {
    struct array* arr = (struct array*) target;

    if (id == -1) {
        return arr->length != 0;
    }

    return arr->capacity - 1 > id;
}

void* array_get(size_t idx, struct array* arr) {
    if (idx >= arr->capacity) {
        printf("Array index is greater than capacity\n");
        abort();
    }

    return (char*) arr->arr + arr->element_size * idx;
}

struct element_info array_first(void* array) {
    struct array* arr = (struct array*) array;
    struct element_info first = {
        .id = 0,
        .ptr = arr->arr, 
    };

    return first;
}

struct element_info array_next(void* array, element_id id) {
    if (!array_has_next(array, id)) {
        printf("Array called next but has_next is false\n");
        abort();
    }

    if (id == -1) { return array_first(array); }

    struct array* arr = (struct array*) array;
    size_t element_size = arr->element_size;
    element_id idx = id + 1;

    struct element_info next = {
        .id = idx,
        .ptr = array_get(idx, arr),
    };

    return next;
}

struct collection array_collection(void) {
    struct collection collection = {
        .first = array_first,
        .next = array_next,
        .has_next = array_has_next,
        .length = array_length,
    };

    return collection;
}

void* array_add(void* elem, struct array* arr) {
    void* dest = array_get(arr->length, arr);
    arr->length += 1;

    return memcpy(dest, elem, arr->element_size);
}

void array_destroy(struct array* array) {
    free(array->arr);
}

struct array array_init(size_t elem_size, size_t capacity) {
    void* arr = malloc(elem_size * capacity);

    struct array array = {
        .arr = arr,
        .capacity = capacity,
        .element_size = elem_size,
        .collection = array_collection(), 
        .length = 0,
    };

    return array;
}

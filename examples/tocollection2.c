#include "../stream.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Collection (Vector) Implementation ---
// (This is the same as the previous 'to_collection.c' example)

struct vector {
    int* data;
    size_t size;
    size_t capacity;
};

void* vector_init() {
    struct vector* vec = malloc(sizeof(struct vector));
    vec->size = 0;
    vec->capacity = 8;
    vec->data = malloc(vec->capacity * sizeof(int));
    printf("--- Collection initialized ---\n");
    return vec;
}

void vector_add(void* element, void* collection) {
    struct vector* vec = (struct vector*)collection;
    int* new_element = (int*)element;
    if (vec->size == vec->capacity) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, vec->capacity * sizeof(int));
    }
    vec->data[vec->size] = *new_element;
    vec->size++;
}

void vector_cleanup(struct vector* vec) {
    if (vec) {
        free(vec->data);
        free(vec);
    }
}

// --- Stream Source (Array) Implementation ---
// (This is the same as the previous 'to_collection.c' example)

struct array_state {
    int* data;
    size_t len;
    size_t idx;
};

void* array_next(void* state) {
    struct array_state* s = (struct array_state*)state;
    if (s->idx >= s->len) {
        return NULL;
    }
    return &s->data[s->idx];
}

void array_increment(void* state) {
    struct array_state* s = (struct array_state*)state;
    s->idx++;
}

// --- Handlers for Operations ---

// Filter
bool is_even(void* element) {
    int* val = (int*)element;
    bool even = (*val % 2) == 0;
    printf("  filter(is_even? %d): %s\n", *val, even ? "KEEP" : "DISCARD");
    return even;
}

// Map
void square_it(void* output_slot, void* input_element) {
    int* in = (int*)input_element;
    int* out = (int*)output_slot;
    *out = (*in) * (*in);
    printf("  map(square %d): %d\n", *in, *out);
}

// Peek
void peek_handler(void* element) {
    // This peek is placed after 'filter' but before 'limit'
    printf("  peek(post-filter): %d\n", *(int*)element);
}

// --- Main Example ---

int main() {
    int my_data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    struct array_state source_state = {
        .data = my_data,
        .len = sizeof(my_data) / sizeof(my_data[0]),
        .idx = 0};

    struct stream s = stream_init(&source_state, array_next, array_increment);

    printf("Building stream pipeline...\n");
    // Pipeline:
    // source -> filter(is_even) -> peek() -> limit(3) -> map(square_it)
    
    stream_filter(&s, is_even);
    stream_peek(&s, peek_handler); // Peek *after* filtering
    stream_limit(&s, 3);           // Limit to 3 elements
    stream_map(&s, square_it, sizeof(int));

    printf("Running stream_to_collection...\n\n");
    
    // Note: stream_to_collection now takes the 'init' and 'add' handlers
    struct vector* result_vec =
        stream_to_collection(&s, vector_init, vector_add);

    printf("\n--- Stream consumed. ---\n");
    printf("Final Collection (%zu items):\n", result_vec->size);
    for (size_t i = 0; i < result_vec->size; i++) {
        printf("  vec[%zu] = %d\n", i, result_vec->data[i]);
    }

    vector_cleanup(result_vec);
    return 0;
}

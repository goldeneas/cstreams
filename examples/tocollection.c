#include "../stream.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief A simple dynamic vector to act as our "collection".
 */
struct vector {
    int* data;
    size_t size;
    size_t capacity;
};

/**
 * @brief The 'init' function for our collection.
 * Matches the 'void* (*init)()' signature.
 */
void* vector_init() {
    struct vector* vec = malloc(sizeof(struct vector));
    if (!vec) {
        perror("Failed to allocate vector");
        exit(1);
    }
    vec->size = 0;
    vec->capacity = 8; // Initial capacity
    vec->data = malloc(vec->capacity * sizeof(int));
    if (!vec->data) {
        perror("Failed to allocate vector data");
        free(vec);
        exit(1);
    }
    printf("Collection initialized.\n");
    return vec;
}

/**
 * @brief The 'add' function for our collection.
 * Matches the 'void (*add)(void* elem, void* collection)' signature.
 */
void vector_add(void* element, void* collection) {
    struct vector* vec = (struct vector*)collection;
    int* new_element = (int*)element;

    // Check if we need to resize
    if (vec->size == vec->capacity) {
        vec->capacity *= 2;
        int* new_data = realloc(vec->data, vec->capacity * sizeof(int));
        if (!new_data) {
            perror("Failed to realloc vector");
            // In a real app, you'd handle this more gracefully
            exit(1);
        }
        vec->data = new_data;
    }

    // Add the new element
    vec->data[vec->size] = *new_element;
    vec->size++;
}

/**
 * @brief Cleans up the vector memory.
 */
void vector_cleanup(struct vector* vec) {
    if (vec) {
        free(vec->data);
        free(vec);
    }
}

// --- Stream Source (from an array) ---

struct array_state {
    int* data;
    size_t len;
    size_t idx;
};

void* array_next(void* state) {
    struct array_state* s = (struct array_state*)state;
    if (s->idx >= s->len) {
        return NULL; // End of stream
    }
    return &s->data[s->idx];
}

void array_increment(void* state) {
    struct array_state* s = (struct array_state*)state;
    s->idx++;
}

// --- Intermediate Operations ---

/**
 * @brief A 'filter_handler' that keeps only even numbers.
 */
bool is_even(void* element) {
    int* val = (int*)element;
    return (*val % 2) == 0;
}

/**
 * @brief A 'map_handler' that squares a number.
 */
void square_it(void* output_slot, void* input_element) {
    int* in = (int*)input_element;
    int* out = (int*)output_slot;
    *out = (*in) * (*in);
}

// --- Main Example ---

int main() {
    int my_data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // 1. Set up the stream source state
    struct array_state source_state = {
        .data = my_data,
        .len = sizeof(my_data) / sizeof(my_data[0]),
        .idx = 0};

    // 2. Initialize the stream
    struct stream s = stream_init(&source_state, array_next, array_increment);

    // 3. Add intermediate operations
    //    Pipeline: [1..10] -> filter(is_even) -> map(square_it)
    //    Result:   [2, 4, 6, 8, 10] -> [4, 16, 36, 64, 100]
    stream_filter(&s, is_even);
    stream_map(&s, square_it, sizeof(int));

    // 4. Run the terminal operation 'stream_to_collection'
    printf("Running stream_to_collection...\n");
    struct vector* result_vec =
        stream_to_collection(&s, vector_init, vector_add);

    // 5. Print the results
    printf("Stream consumed. Resulting collection (%zu items):\n", result_vec->size);
    for (size_t i = 0; i < result_vec->size; i++) {
        printf("  vec[%zu] = %d\n", i, result_vec->data[i]);
    }

    // 6. Clean up the collection
    vector_cleanup(result_vec);

    return 0;
}

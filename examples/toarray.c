#include "../stream.h"
#include <stdio.h>
#include <stdlib.h>

// --- 1. Define a state for our array iterator ---
// This struct will hold the source array and current position
struct array_iterator {
    int* array;
    size_t length;
    size_t current_index;
};

// --- 2. Implement stream_init handlers ---

/**
 * next_handler: Returns a pointer to the current element or NULL if finished.
 */
void* iterator_next(void* state) {
    struct array_iterator* iter = (struct array_iterator*)state;
    if (iter->current_index >= iter->length) {
        return NULL; // End of stream
    }
    // Return a pointer to the element at the current index
    return &iter->array[iter->current_index];
}

/**
 * increment_state_handler: Moves the iterator to the next position.
 */
void iterator_increment(void* state) {
    struct array_iterator* iter = (struct array_iterator*)state;
    iter->current_index++;
}

// --- 3. Implement operation handlers ---

/**
 * filter_handler: Keeps only even numbers.
 */
bool filter_even(void* src) {
    int* num = (int*)src;
    return (*num % 2) == 0;
}

/**
 * map_handler: Converts an 'int' to its 'long' square.
 *
 * src: void* (points to an int)
 * dst: void* (points to the long output_slot)
 */
void map_to_long_square(void* dst, void* src) {
    long* output_slot = (long*)dst;
    int* input_num = (int*)src;
    
    // Calculate the square and store it in the destination
    *output_slot = (long)(*input_num) * (long)(*input_num);
}


// --- 4. Main function to run the stream ---
int main(void) {
    // Our source data
    int numbers[] = {1, 2, 3, 4, 5, 6};
    
    // Initialize the iterator state
    struct array_iterator iter_state = {
        .array = numbers,
        .length = 6,
        .current_index = 0
    };

    // 1. Initialize the stream
    struct stream stream = stream_init(&iter_state, iterator_next, iterator_increment);

    // 2. Add a filter operation
    stream_filter(&stream, filter_even);

    // 3. Add a map operation
    // We are mapping from int -> long, so the output size is sizeof(long)
    stream_map(&stream, map_to_long_square, sizeof(long));

    // 4. Prepare destination array and run the terminal operation
    // We expect 3 results (2*2, 4*4, 6*6)
    long results[3];
    
    // We *must* tell stream_to_array the size of the *final* elements.
    // Our map operation produces 'long', so we pass sizeof(long).
    stream_to_array(&stream, results, sizeof(long));


    // 5. Print the results
    printf("Stream processing complete. Results:\n");
    size_t result_count = sizeof(results) / sizeof(long);

    for (size_t i = 0; i < result_count; i++) {
        // We expect: 4, 16, 36
        printf("Result %zu: %ld\n", i, results[i]);
    }

    return 0;
}

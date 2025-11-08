#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../stream.h"

/**
 * @brief The state for our array-based stream.
 * It holds a pointer to the data, the total size,
 * and the current processing index.
 */
struct array_iterator {
    int* data;
    int size;
    int index;
};

// --- 1. Stream Source Handlers ---

/**
 * @brief The 'next_handler' implementation.
 * It returns a pointer to the current int, or NULL if the stream is empty.
 */
void* get_next_int(void* state) {
    struct array_iterator* iter = (struct array_iterator*)state;
    if (iter->index >= iter->size) {
        return NULL; // Signal end of stream
    }
    // Return a pointer to the data at the current index
    return (void*)&iter->data[iter->index];
}

/**
 * @brief The 'increment_state_handler' implementation.
 * It moves the iterator to the next element.
 */
void increment_int_iterator(void* state) {
    struct array_iterator* iter = (struct array_iterator*)state;
    iter->index++;
}


// --- 2. Stream Operation Handlers ---

/**
 * @brief A 'filter_handler' that returns true only for even numbers.
 */
bool is_even(void* src) {
    int val = *(int*)src;
    return (val % 2) == 0;
}

/**
 * @brief A 'map_handler' that squares an integer.
 * 'dst' is the output_slot, 'src' is the element from the stream.
 */
void square_it(void* dst, void* src) {
    int val = *(int*)src;
    *(int*)dst = val * val; // Write the result to the output slot
}

// --- 3. Main Program ---

int main() {
    // Our source data
    int source_array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int source_size = sizeof(source_array) / sizeof(int);

    printf("Source data: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]\n");
    printf("Original array size: %d\n", source_size);
    
    // 1. Initialize the stream state
    struct array_iterator iter_state = {
        .data = source_array,
        .size = source_size,
        .index = 0
    };

    // 2. Initialize the stream
    // We pass our iterator state and the two handler functions.
    struct stream s = stream_init(
        &iter_state, 
        get_next_int, 
        increment_int_iterator
    );

    printf("\nBuilding pipeline: filter(is_even) -> map(square_it)\n");

    // 3. Build the pipeline
    
    // Add a filter operation (will reduce the count)
    stream_filter(&s, is_even);
    
    // Add a map operation (will not reduce the count)
    stream_map(&s, square_it, sizeof(int));

    // 4. Run the new terminal operation
    printf("Calling stream_count()...\n");

    // This will consume the stream and call stream_cleanup() internally.
    size_t final_count = stream_count(&s);

    printf("Count of elements after filter: %zu\n", final_count);

    // At this point, 's' is "consumed" and its internal data
    // has been freed by stream_count.
    // You must not use 's' again.

    printf("\n...Stream consumed.\n");

    return 0;
}

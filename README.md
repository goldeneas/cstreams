# CStreams

A simple, Java-inspired Streams API for C.

This library provides a basic framework for processing sequences of data using a pipeline of operations, similar to Java's `Stream` API.

## Features

* **Filter**: Filter elements based on a predicate.
* **Map**: Transform elements from one value to another.
* **For Each**: A terminal operation to consume the stream.
* **Extensible**: Use any data source by providing two simple functions.

## How To Build
This is a library, not a standalone executable; to use it, you just need to compile your main.c with cstreams.

## How To Use

First, you need a data source. This can be an array, a list, a file, etc. You just need to provide two functions: `next_handler` and `increment_state_handler`.

Here is a complete example that processes an array of integers:

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stream.h"

// --- 1. Define the data source state ---
struct array_iterator {
    int* data;
    int size;
    int index;
};

// --- 2. Implement the source handlers ---

// Gets the next element, or NULL if done
void* get_next_int(void* state) {
    struct array_iterator* iter = (struct array_iterator*)state;
    if (iter->index >= iter->size) {
        return NULL; // End of stream
    }
    return (void*)&iter->data[iter->index];
}

// Moves to the next element
void increment_int_iterator(void* state) {
    struct array_iterator* iter = (struct array_iterator*)state;
    iter->index++;
}

// --- 3. Implement operation handlers ---

// Filter
bool is_even(void* src) {
    int val = *(int*)src;
    return (val % 2) == 0;
}

// Map
void square_it(void* dst, void* src) {
    int val = *(int*)src;
    *(int*)dst = val * val;
}

// ForEach
void print_it(void* src) {
    printf("Result: %d\n", *(int*)src);
}

// --- 4. Main Program ---
int main() {
    int source_array[] = {1, 2, 3, 4, 5, 6};
    
    struct array_iterator iter_state = {
        .data = source_array,
        .size = 6,
        .index = 0
    };

    // 1. Initialize the stream
    struct stream s = stream_init(
        &iter_state, 
        get_next_int, 
        increment_int_iterator
    );

    // 2. Build the pipeline
    stream_filter(&s, is_even);         // Keep only 2, 4, 6
    stream_map(&s, square_it, sizeof(int)); // Transform to 4, 16, 36

    // 3. Run the terminal operation
    // This consumes the stream and prints the results
    stream_for_each(&s, print_it);

    // Output:
    // Result: 4
    // Result: 16
    // Result: 36

    return 0;
}

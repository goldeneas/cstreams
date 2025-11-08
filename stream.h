#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef void(*func_ptr)(void);
typedef void(*map_handler)(void* dst, void* src);
typedef bool(*filter_handler)(void* src);

typedef void(*foreach_handler)(void* src);

typedef void* (*next_handler)(void* state);
typedef void (*increment_state_handler)(void* state);

struct stream_op_node {
    struct stream_op* op;
    struct stream_op_node* next;
};

struct stream{
    void* state;
    next_handler next;
    increment_state_handler increment_state;

    struct stream_op_node* ops;
};

struct stream_op {
    void* op_state;
    void* (*process)(void* curr, void* op_state);
    void (*cleanup)(void* op_state);
};

struct stream stream_init(void* state, next_handler next, increment_state_handler increment_state);

void stream_map(struct stream* stream, map_handler handler, size_t output_element_size);
void stream_filter(struct stream* stream, filter_handler handler);

void stream_for_each(struct stream* stream, foreach_handler handler);
void stream_to_array(struct stream* stream, void* array, size_t elem_size);

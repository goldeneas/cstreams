#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef void(*func_ptr)(void);
typedef void(*map_handler)(void* dst, void* src);
typedef bool(*filter_handler)(void* src);

typedef void(*foreach_handler)(void* src);

struct stream{
    void* state;
    void* (*next)(void* state);

    void (*increment_state)(void* state);
};

struct stream_op {
    void* op_state;
    void* (*process)(void* curr, void* op_state);
    void (*cleanup)(void* op_state);
};

struct stream_op* stream_map(struct stream_op* downstream, map_handler handler,
        size_t output_element_size);
struct stream_op* stream_filter(struct stream_op* downstream, filter_handler handler);

void stream_for_each(struct stream* stream, struct stream_op* first, foreach_handler handler);


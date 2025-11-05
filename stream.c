#include "stream.h"
#include <printf.h>
#include <stdlib.h>

struct map_state {
    size_t output_element_size;
    void* output_slot;
};

// downstream can be null
// state is malloc-ed
// stream op is malloc-ed
struct stream_op* stream_map(struct stream_op* downstream, map_handler handler,
        size_t output_element_size) {
    struct map_state* state = malloc(sizeof(struct map_state));
    state->output_element_size = output_element_size;
    state->output_slot = malloc(output_element_size);

    struct stream_op* op = malloc(sizeof(struct stream_op));
    op->downstream = downstream;
    op->op_state = state;
    op->handler = (func_ptr) handler;

    return op;
} 

// downstream can be null
// stream op is malloc-ed
struct stream_op* stream_filter(struct stream_op* downstream, filter_handler handler) {
    struct stream_op* op = malloc(sizeof(struct stream_op));
    op->downstream = downstream;
    op->op_state = NULL;
    op->handler = (func_ptr) handler;

    return op;
}

void stream_for_each(struct stream* stream, struct stream_op* first, foreach_handler handler) {
    if (!stream || !first || !handler) {
        printf("Something wrong happened!\n");
        return;
    }

    void* curr = stream->next(stream->state);
    while(curr != NULL) {
        first->handler();
    }
}

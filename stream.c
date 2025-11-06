#include "stream.h"
#include <printf.h>
#include <stdio.h>
#include <stdlib.h>

struct map_state {
    void* output_slot;
    map_handler handler;
};

void stream_op_cleanup(struct stream_op* op) {
    op->cleanup(op->op_state);
    free(op->op_state);
    free(op);
}

void* stream_map_process(void* curr, void* op_state) {
    struct map_state* state = (struct map_state*) op_state;
    map_handler handler = state->handler;

    handler(state->output_slot, curr);
    return state->output_slot;
}

// downstream can be null
// state is malloc-ed
// stream op is malloc-ed
struct stream_op* stream_map(struct stream_op* downstream, map_handler handler,
        size_t output_element_size) {
    struct map_state* state = malloc(sizeof(struct map_state));
    state->output_slot = malloc(output_element_size);
    state->handler = handler;

    struct stream_op* op = malloc(sizeof(struct stream_op));
    op->op_state = state;
    op->process = stream_map_process;

    return op;
} 

struct filter_state {
    filter_handler handler;
};

void* stream_filter_process(void* curr, void* op_state) {
    struct filter_state* state = (struct filter_state*) op_state;
    filter_handler handler = state->handler;

    bool should_keep = handler(curr);
    return should_keep ? curr : NULL;
}

// downstream can be null
// stream op is malloc-ed
struct stream_op* stream_filter(struct stream_op* downstream, filter_handler handler) {
    struct filter_state* state = malloc(sizeof(struct filter_state));
    state->handler = handler;

    struct stream_op* op = malloc(sizeof(struct stream_op));
    op->op_state = state;
    op->process = stream_filter_process;

    return op;
}

void stream_for_each(struct stream* stream, struct stream_op* op, foreach_handler handler) {
    if (!stream || !op || !handler) {
        printf("Something wrong happened!\n");
        return;
    }

    void* curr = NULL;

    do {
        curr = stream->next(stream->state);

        // here we need to apply all processing methods from
        // stream_ops associated with that stream
        void* result = op->process(curr, op->op_state);
        handler(result);

        // TODO: I dont like this function
        stream->increment_state(stream->state);
    } while(curr != NULL);

    stream_op_cleanup(op);
}

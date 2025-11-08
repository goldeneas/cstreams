#include "stream.h"
#include <stdio.h>
#include <stdlib.h>

struct map_state {
    void* output_slot;
    map_handler handler;
};

void stream_op_cleanup(struct stream_op* op) {
    if (op->cleanup) {
        op->cleanup(op->op_state);
    }

    free(op->op_state);
    free(op);
}

void stream_cleanup(struct stream* stream) {
    struct stream_op_node* next = NULL;
    struct stream_op_node* curr = stream->ops;

    while (curr != NULL) {
        stream_op_cleanup(curr->op);
        next = curr->next;
        
        free(curr);
        curr = next;
    }
}

struct stream stream_init(void* state, next_handler next,
        increment_state_handler increment_state) {
    return (struct stream) {
        .state = state,
        .increment_state = increment_state,
        .next = next,
        .ops = NULL
    };
}

void stream_append_op(struct stream* stream, struct stream_op* op) {
    struct stream_op_node* node = malloc(sizeof(struct stream_op_node)); 
    node->next = NULL;
    node->op = op;

    if (stream->ops == NULL) {
        stream->ops = node;
        return;
    }

    struct stream_op_node* curr = stream->ops;
    while (curr->next != NULL) {
        curr = curr->next;
    }

    curr->next = node;
}

void stream_map_cleanup(void* state) {
    struct map_state* s = (struct map_state*) state;
    free(s->output_slot);
}

void* stream_map_process(void* curr, void* op_state) {
    struct map_state* state = (struct map_state*) op_state;
    map_handler handler = state->handler;

    handler(state->output_slot, curr);
    return state->output_slot;
}

void stream_map(struct stream* stream, map_handler handler,
        size_t output_element_size) {
    struct map_state* state = malloc(sizeof(struct map_state));
    state->output_slot = malloc(output_element_size);
    state->handler = handler;

    struct stream_op* op = malloc(sizeof(struct stream_op));
    op->op_state = state;
    op->process = stream_map_process;
    op->cleanup = stream_map_cleanup;

    stream_append_op(stream, op);
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

void stream_filter(struct stream* stream, filter_handler handler) {
    struct filter_state* state = malloc(sizeof(struct filter_state));
    state->handler = handler;

    struct stream_op* op = malloc(sizeof(struct stream_op));
    op->op_state = state;
    op->process = stream_filter_process;
    op->cleanup = NULL;

    stream_append_op(stream, op);
}

void* stream_process_element(void* elem, struct stream* stream) {
    if (!elem || !stream) { return NULL; }

    void* result = elem;
    struct stream_op_node* curr = stream->ops;

    while (curr != NULL) {
        struct stream_op* op = curr->op;
        result = op->process(result, op->op_state);
        
        if (result == NULL) { break; }

        curr = curr->next;
    }

    return result;
}

void stream_for_each(struct stream* stream, foreach_handler handler) {
    if (!stream || !handler) { return; }

    void* elem = stream->next(stream->state);
    while (elem != NULL) {
        void* result = stream_process_element(elem, stream);
        if (result == NULL) { continue; }

        handler(result);

        stream->increment_state(stream->state);
        elem = stream->next(stream->state);
    }

    stream_cleanup(stream);
}

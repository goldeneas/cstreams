#include "stream.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Generic stream handling functions

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

// INTERMEDIATE OPERATIONS

// map functions

struct map_state {
    void* output_slot;
    map_handler mapper;
};

void stream_map_cleanup(void* state) {
    struct map_state* s = (struct map_state*) state;
    free(s->output_slot);
}

void* stream_map_process(void* curr, void* op_state) {
    struct map_state* state = (struct map_state*) op_state;
    map_handler handler = state->mapper;

    handler(state->output_slot, curr);
    return state->output_slot;
}

void stream_map(struct stream* stream, map_handler handler,
        size_t output_element_size) {
    struct map_state* state = malloc(sizeof(struct map_state));
    state->output_slot = malloc(output_element_size);
    state->mapper = handler;

    struct stream_op* op = malloc(sizeof(struct stream_op));
    op->op_state = state;
    op->process = stream_map_process;
    op->cleanup = stream_map_cleanup;

    stream_append_op(stream, op);
} 

// filter functions

struct filter_state {
    filter_handler filter;
};

void* stream_filter_process(void* curr, void* op_state) {
    struct filter_state* state = (struct filter_state*) op_state;
    filter_handler handler = state->filter;

    bool should_keep = handler(curr);
    return should_keep ? curr : NULL;
}

void stream_filter(struct stream* stream, filter_handler handler) {
    struct filter_state* state = malloc(sizeof(struct filter_state));
    state->filter = handler;

    struct stream_op* op = malloc(sizeof(struct stream_op));
    op->op_state = state;
    op->process = stream_filter_process;
    op->cleanup = NULL;

    stream_append_op(stream, op);
}

// UTIL FUNCTIONS

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

void stream_consume(struct stream* stream, void (*consumer)(void* result, void* ctx), void* ctx) {
    if (!stream || !consumer) { return; }

    void* elem = stream->next(stream->state);
    while (elem != NULL) {
        void* result = stream_process_element(elem, stream);

        if (result != NULL) {
            consumer(result, ctx);
        }

        stream->increment_state(stream->state);
        elem = stream->next(stream->state);
    }

    stream_cleanup(stream);
}

// TERMINAL OPERATIONS

struct foreach_ctx {
    foreach_handler handler;
};

void _foreach_consume(void* element, void* ctx) {
    struct foreach_ctx* c = (struct foreach_ctx*) ctx;
    c->handler(element);
}

void stream_for_each(struct stream* stream, foreach_handler handler) {
    struct foreach_ctx ctx = {
        .handler = handler,
    };

    stream_consume(stream, _foreach_consume, &ctx);
}

// to_array

struct to_array_ctx {
    size_t elem_size;
    void* array;
    size_t idx;
};

void _to_array_consume(void* element, void* ctx) {
    struct to_array_ctx* c = (struct to_array_ctx*) ctx;
    size_t* idx = &c->idx;
    void* array = c->array;
    size_t elem_size = c->elem_size;

    void* dst = (char*) array + (*idx) * elem_size;
    memcpy(dst, element, elem_size);
    *idx += 1;
}

void stream_to_array(struct stream* stream, void* array, size_t elem_size) {
    struct to_array_ctx ctx = {
        .elem_size = elem_size,
        .array = array,
        .idx = 0,
    };

    stream_consume(stream, _to_array_consume, &ctx);
}

// to_collection

struct to_collection_ctx {
    void* collection;
    void (*add)(void* element, void* collection);
};

void _to_collection_consume(void* element, void* ctx) {
    struct to_collection_ctx* c = (struct to_collection_ctx*) ctx;
    c->add(element, c->collection);
}

void* stream_to_collection(struct stream* stream, void* (*init)(),
        void (*add)(void* elem, void* collection)) {
    void* collection = init();

    struct to_collection_ctx ctx = {
        .collection = collection,
        .add = add,
    };

    stream_consume(stream, _to_collection_consume, &ctx);
    return collection;
}

// count

void _count_consumer(void* element, void* state) {
    size_t* count = (size_t*) state;
    *count += 1;
}

size_t stream_count(struct stream* stream) {
    size_t count = 0;
    stream_consume(stream, _count_consumer, &count);

    return count;
}   

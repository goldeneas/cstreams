#include "stream.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef bool (*stream_consumer)(void* element, void* ctx);

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
        .ops = NULL,
        .tail = NULL,
    };
}

void stream_append_op(struct stream* stream, struct stream_op* op) {
    struct stream_op_node* node = malloc(sizeof(struct stream_op_node)); 
    node->next = NULL;
    node->op = op;

    if (stream->ops == NULL) {
        stream->ops = node;
        stream->tail = node;
        return;
    }

    stream->tail->next = node;
    stream->tail = node;
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

// limit functions

struct limit_state {
    size_t length;
    size_t max_length;
};

void* stream_limit_process(void* curr, void* op_state) {
    struct limit_state* state = (struct limit_state*) op_state;
    if (state->length >= state->max_length) {
        return NULL;
    }

    state->length += 1;
    return curr;
}

void stream_limit(struct stream* stream, size_t max_length) {
    struct limit_state* state = malloc(sizeof(struct limit_state));
    state->length = 0;
    state->max_length = max_length;

    struct stream_op* op = malloc(sizeof(struct stream_op));
    op->op_state = state;
    op->process = stream_limit_process;
    op->cleanup = NULL;

    stream_append_op(stream, op);
}

// peek functions

struct peek_state {
    void (*peek_handler)(void* element);
};

void* stream_peek_process(void* curr, void* op_state) {
    struct peek_state* state = (struct peek_state*) op_state;
    state->peek_handler(curr);
    return curr;
}

void stream_peek(struct stream* stream, void (*peek_handler)(void* element)) {
    struct peek_state* state = malloc(sizeof(struct peek_state));
    state->peek_handler = peek_handler;

    struct stream_op* op = malloc(sizeof(struct stream_op));
    op->op_state = state;
    op->process = stream_peek_process;
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

void stream_consume(struct stream* stream, stream_consumer consumer, void* ctx) {
    if (!stream || !consumer) { return; }

    void* elem = stream->next(stream->state);
    while (elem != NULL) {
        void* result = stream_process_element(elem, stream);

        if (result != NULL) {
            bool should_continue = consumer(result, ctx);
            if (!should_continue) {
                break;
            }
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

bool _foreach_consume(void* element, void* ctx) {
    struct foreach_ctx* c = (struct foreach_ctx*) ctx;
    c->handler(element);

    return true;
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

bool _to_array_consume(void* element, void* ctx) {
    struct to_array_ctx* c = (struct to_array_ctx*) ctx;
    size_t* idx = &c->idx;
    void* array = c->array;
    size_t elem_size = c->elem_size;

    void* dst = (char*) array + (*idx) * elem_size;
    memcpy(dst, element, elem_size);
    *idx += 1;

    return true;
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

bool _to_collection_consume(void* element, void* ctx) {
    struct to_collection_ctx* c = (struct to_collection_ctx*) ctx;
    c->add(element, c->collection);

    return true;
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

bool _count_consumer(void* element, void* ctx) {
    (void) element;

    size_t* count = (size_t*) ctx;
    *count += 1;
    return true;
}

size_t stream_count(struct stream* stream) {
    size_t count = 0;
    stream_consume(stream, _count_consumer, &count);

    return count;
}

// any_match

struct any_match_ctx {
    match_predicate predicate;
    bool match;
};

bool _any_match_consumer(void* element, void* ctx) {
    struct any_match_ctx* c = (struct any_match_ctx*) ctx;

    if (!c->predicate(element)) { return true; }

    c->match = true;
    return false;
}

bool stream_any_match(struct stream* stream, match_predicate predicate) {
    struct any_match_ctx ctx = {
        .match = false,
        .predicate = predicate,
    };

    stream_consume(stream, _any_match_consumer, &ctx);
    return ctx.match;
}

// any_match

struct all_match_ctx {
    match_predicate predicate;
    bool match;
};

bool _all_match_consumer(void* element, void* ctx) {
    struct all_match_ctx* c = (struct all_match_ctx*) ctx;

    if (c->predicate(element)) { return true; }

    c->match = false;
    return false;
}

bool stream_all_match(struct stream* stream, match_predicate predicate) {
    struct all_match_ctx ctx = {
        .match = true,
        .predicate = predicate,
    };

    stream_consume(stream, _all_match_consumer, &ctx);
    return ctx.match;
}

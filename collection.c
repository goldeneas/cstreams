#include "collection.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct element_info element_info_empty(void) {
    return (struct element_info) {
        .id = NULL,
        .ptr = NULL,
    };
}

void collection_for_each(struct collection* collection, void* target, foreach_handler handler) {
    if (!collection->has_next(target, NULL)) { return; }

    struct element_info curr = element_info_empty(); 
    while (collection->has_next(target, &curr.id)) {
        curr = collection->next(target, &curr.id);
        handler(curr.ptr);
    }
}

void* to_array(struct mapped_collection* mcollection, void* target) {
    struct collection* collection = mcollection->collection;

    size_t length = collection->length(target);
    if (length <= 0) { return NULL; }
    if (!collection->has_next(target, NULL)) { return NULL; }

    map_handler mapper = mcollection->mapper;
    void* arr = malloc(mcollection->mapped_type_size * length);

    size_t i = 0;
    struct element_info curr = element_info_empty(); 
    while (collection->has_next(target, &curr.id)) {
        curr = collection->next(target, &curr.id);

        void* mapped_ptr = (char*) arr + i * mcollection->mapped_type_size; 
        mapper(curr.ptr, mapped_ptr);

        i += 1;
    }

    return arr;
}

struct mapped_collection collection_map(struct collection* collection, void* target, map_handler mapper,
            size_t mapped_type_size) {
    return (struct mapped_collection) {
        .collection = collection,
        .target = target,
        .mapper = mapper,
        .mapped_type_size = mapped_type_size,
    };
}

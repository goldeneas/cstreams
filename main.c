#include "array.h"
#include "collection.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void handler(void* element) {
    printf("Element is: %p\n", element);
}

void mapper(void* element, void* dst) {
    int32_t* ptr = element;
}

int main(void) {
    struct array a = array_init(sizeof(int32_t), 3);

    int32_t b = 1, c = 3, d = 5;
    array_add(&b, &a);
    array_add(&c, &a);
    array_add(&d, &a);

    printf("Array has size %zu/%zu\n", a.length, a.capacity);

    collection_for_each(&a.collection, &a, handler);

    struct mapped_collection mcol = collection_map(&a.collection, &a, mapper, sizeof(int32_t));
    int32_t* arr = collection_to_array(&mcol);

    for (int i = 0; i < 3; i++) {
        printf("Elem is %d\n", arr[i]);
    }

    return 0;
}

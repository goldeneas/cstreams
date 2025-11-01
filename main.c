#include "array.h"
#include "collection.h"
#include <stdint.h>
#include <stdio.h>

void handler(void* element) {
    printf("HELLO: %p\n", element);
}

int main(void) {
    struct array a = array_init(sizeof(int32_t), 3);

    int32_t b = 1, c = 3, d = 5;
    array_add(&b, &a);
    array_add(&c, &a);
    array_add(&d, &a);

    printf("Array has size %zu/%zu\n", a.length, a.capacity);

    collection_for_each(&a.collection, &a, handler);

    return 0;
}

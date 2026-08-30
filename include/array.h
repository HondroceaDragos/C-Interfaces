#ifndef ARRAY_H
#define ARRAY_H

#include "./helpers.h"
#include "./iterator.h"

static inline void *_array_next(Iterator i, size_t esize) {
    return (int8_t *)i->ref + esize;
}

#define ArrayType(type) \
    typedef struct concat_layer2(_array_, type) { \
        type *data; \
        size_t size; \
    } concat_layer2(Array_, type); \
    deleteType(Array(type))

#define Array(type) concat_layer2(Array_, type)
#define newArray(type, ...) (Array(type)){(type[])__VA_ARGS__, .size = sizeof((type[])__VA_ARGS__) / sizeof(type)}

#define forArray_primitive(i, once, arr, acc) \
    for (Iterator i = newIterator((arr).data, (arr).size); i && i->size; iterator_advance(&i, _array_next, sizeof(*(arr).data))) \
        for (acc = (typeof(*(arr).data) *)i->ref, *once = (void *)1; once; once = 0)

#define forArray(acc, arr) \
    forArray_primitive(concat_layer2(_i_, __COUNTER__), concat_layer2(_once_, __COUNTER__), arr, acc)

#endif

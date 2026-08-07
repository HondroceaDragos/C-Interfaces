#ifndef ARRAY_H
#define ARRAY_H

#include "./helpers.h"

#define Array(type) concat_layer2(Array_, type)

#define ArrayType(type) \
    typedef struct concat_layer2(_array_, type) { \
        type *data; \
        size_t size; \
    } concat_layer2(Array_, type);

#define newArray(type, ...) (Array(type)){(type[])__VA_ARGS__, .size = sizeof((type[])__VA_ARGS__) / sizeof(type)}

#endif
#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "./fortype.h"

typedef enum : int8_t {
    RETAIN_MEMORY,
    FREE_MEMORY
} MemoryCleanup;

#define notfound -1

#define concat_layer1(a, b) a##b
#define concat_layer2(a, b) concat_layer1(a, b)

#define defer(func) [[gnu::cleanup(func)]]

#define deleteDefine(type) static inline void concat_layer2(_delete_, type)(type *self)
#define deleteType(type) static inline void concat_layer2(_delete_, type)(type *obj) {}
#define delete(type) concat_layer2(_delete_, type)

#endif

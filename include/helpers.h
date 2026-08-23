#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "./fortype.h"
#include "./stdtypes.h"

typedef int32_t (*CmpFunc)(const void *, const void *);
typedef int32_t (*HashFunc)(const void *);

typedef enum : int8_t {
    RETAIN_MEMORY,
    FREE_MEMORY
} MemoryCleanup;

#define notfound -1

#define concat_layer1(a, b) a##b
#define concat_layer2(a, b) concat_layer1(a, b)

#define instanceof(x, y) _Generic(*(typeof(x) *)0, \
    typeof_unqual(y): true, \
    default: false \
)

#define swap(a, b) \
    do { \
        if (!instanceof(a, b)) raise(WARNING, "Swap " YELLOW "operands do not match" RESET ". Elements will remain unchanged."); \
        auto _tmp = (a); \
        (a) = (b); \
        (b) = _tmp; \
    } while (false);

#define defer(func) [[gnu::cleanup(func)]]

#define $override(vtable, func, new) \
    static typeof(*vtable.func) new; \
    [[gnu::constructor]] \
    static void concat_layer2(_override_accept_, new)(void) { \
        vtable.func = new; \
    }

#define deleteDefine(type) static inline void concat_layer2(_delete_, type)(type *self)
#define deleteType(type) static inline void concat_layer2(_delete_, type)(type *obj) {}
#define delete(type) concat_layer2(_delete_, type)

#endif

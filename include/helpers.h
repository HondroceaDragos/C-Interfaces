#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "./fortype.h"
#include "./stdtypes.h"
#include "./raise.h"

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

#define debug(v) \
    do { \
        i8 *s = (v)->toString((v)); \
        printf("%s\n", s); \
        free(s); \
    } while (false);

static inline i8 *cstrfmt(const i8* fmt, ...) {
    va_list args;
    va_list tmp;

    va_start(args, fmt);
    va_copy(tmp, args);

    i32 slen = vsnprintf(nullptr, 0, fmt, tmp);
    va_end(tmp);

    if (slen < 0) {
        va_end(args);
        raise(ERROR, "Cannot " RED "create cstrfmt " RESET "(out-of-memory).");
    }

    slen += 1;
    i8 *data = calloc(slen, sizeof(*data));
    if (!data) {
        va_end(args);
        raise(ERROR, "Cannot " RED "create cstrfmt " RESET "(out-of-memory).");
    }

    vsnprintf(data, slen, fmt, args);

    return data;
}

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

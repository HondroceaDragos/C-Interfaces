#ifndef VTABLE_TUPLE_H
#define VTABLE_TUPLE_H

#include "./helpers.h"
#include "./raise.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#define TupleVTableFunctions(id, fst, snd) \
    int8_t *(*fmt)(const fst, const snd); \
    int8_t *(*toString)(id); \

#define TupleVTableType(id, fst, snd) \
    typedef struct concat_layer2(_tuple_vtable_, concat_layer2(fst, snd)) { \
        TupleVTableFunctions(id, fst, snd) \
    } concat_layer2(TupleVTable_, concat_layer2(fst, snd)); \
    static inline int8_t *concat_layer2(_tuple_default_fmt_, concat_layer2(fst, snd))(const fst, const snd) { \
        raise(WARNING, "Tuple uses " YELLOW "default formatting function " RESET "(cannot deduce type)."); \
        return strdup("[?]"); \
    } \
    static inline void delete(id)(id *t) { \
        if (!t || !*t) return; \
        delete(fst)(&(*t)->first); \
        delete(snd)(&(*t)->second); \
        free(*t); \
        *t = nullptr; \
    } \
    static inline int8_t *concat_layer2(_tuple_default_tostring_, concat_layer2(fst, snd))(id self) { \
        size_t bcap = 64; \
        \
        int8_t *buffer = calloc(bcap, sizeof(*buffer)); \
        buffer[0] = '\0'; \
        \
        int8_t *fmt = self->fmt(self->first, self->second); \
        snprintf(buffer, bcap, "Tuple(%s)", fmt); \
        free(fmt); \
        return buffer; \
    } \
    static concat_layer2(TupleVTable_, concat_layer2(fst, snd)) concat_layer2(TupleVTable_, concat_layer2(tuple_, concat_layer2(fst, snd))) = { \
        .fmt = concat_layer2(_tuple_default_fmt_, concat_layer2(fst, snd)), \
        .toString = concat_layer2(_tuple_default_tostring_, concat_layer2(fst, snd)), \
    }; \

#define TupleVTable(fst, snd) concat_layer2(TupleVTable_, concat_layer2(fst, snd))
#define TupleVTableInstance(fst, snd) concat_layer2(TupleVTable_, concat_layer2(tuple_, concat_layer2(fst, snd)))

#endif

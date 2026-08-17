#ifndef TUPLE_H
#define TUPLE_H

#include "./vtable_tuple.h"

#define TupleType(fst, snd) \
    typedef struct concat_layer2(_tuple_, concat_layer2(fst, snd)) { \
        fst first; \
        snd second; \
        \
        TupleVTableFunctions(struct concat_layer2(_tuple_, concat_layer2(fst, snd)) *, fst, snd) \
    } *concat_layer2(Tuple_, concat_layer2(fst, snd)); \
    \
    TupleVTableType(concat_layer2(Tuple_, concat_layer2(fst, snd)), fst, snd) \
    \
    static inline concat_layer2(Tuple_, concat_layer2(fst, snd)) concat_layer2(newTuple_, concat_layer2(fst, snd))( \
        struct concat_layer2(_tuple_, concat_layer2(fst, snd)) defaults) { \
        concat_layer2(Tuple_, concat_layer2(fst, snd)) t = calloc(1, sizeof(*t)); \
        if (!t) raise(ERROR, "Cannot " RED "create tuple " RESET "(out-of-memory)."); \
        \
        t->first = defaults.first; \
        t->second = defaults.second; \
        \
        t->fmt = (defaults.fmt) ? defaults.fmt : TupleVTableInstance(fst, snd).fmt; \
        t->toString = (defaults.toString) ? defaults.toString : TupleVTableInstance(fst, snd).toString; \
        \
        return t;  \
    }

#define Tuple(fst, snd) concat_layer2(Tuple_, concat_layer2(fst, snd))
#define newTuple(fst, snd, ...) concat_layer2(newTuple_, concat_layer2(fst, snd))((struct concat_layer2(_tuple_, concat_layer2(fst, snd))){__VA_ARGS__})

#endif
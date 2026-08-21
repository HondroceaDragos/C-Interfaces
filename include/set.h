#ifndef SET_H
#define SET_H

#include "./array.h"
#include "./vtable_set.h"
#include "./raise.h"
#include "./iterator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void *_set_next(Iterator i, size_t esize) {
    return (int8_t *)i->ref + esize;
}

#define SetType(type) \
    typedef struct concat_layer2(_set_, type) { \
        type *data; \
        size_t capacity; \
        size_t size; \
        CmpFunc cmp; \
        \
        SetVTableFunctions(struct concat_layer2(_set_, type) *, type) \
    } *concat_layer2(Set_, type); \
    \
    SetVTableType(concat_layer2(Set_, type), type) \
    \
    struct concat_layer2(_set_defaults_, type) { \
        size_t capacity; \
        Array(type) using; \
        SetVTableFunctions(concat_layer2(Set_, type), type) \
    }; \
    \
    static inline concat_layer2(Set_, type) concat_layer2(newSet_, type)(CmpFunc cmp, struct concat_layer2(_set_defaults_, type) defaults) { \
        concat_layer2(Set_, type) s = calloc(1, sizeof(*s)); \
        if (!s) { \
            raise(ERROR, "Cannot create set " RED "(out-of-memory)" RESET ". Returning nullptr."); \
            return nullptr; \
        } \
        \
        s->capacity = (defaults.capacity < defaults.using.size) ? defaults.using.size : defaults.capacity; \
        if (!s->capacity) s->capacity = SET_DEFAULT_CAPACITY; \
        s->data = calloc(s->capacity, sizeof(type)); \
        if (!s->data) { \
            free(s); \
            raise(ERROR, "Cannot create set " RED "(out-of-memory)" RESET ". Returning nullptr."); \
            return nullptr; \
        } \
        \
        s->cmp = cmp; \
        s->filter = (defaults.filter) ? defaults.filter : SetVTableInstance(type).filter; \
        \
        if (defaults.using.data) { \
            memcpy(s->data, defaults.using.data, defaults.using.size * sizeof(type)); \
            s->size = defaults.using.size; \
            qsort(s->data, s->size, sizeof(type), s->cmp); \
            \
            size_t ulen = 0; \
            for (size_t idx = 0; idx < s->size; idx++) { \
                if (s->filter) if (!s->filter(s->data[idx])) continue; \
                if (s->cmp(&s->data[idx], &s->data[idx + 1])) s->data[ulen++] = s->data[idx]; \
            } \
            s->size = ulen; \
        } \
        \
        s->fmt = (defaults.fmt) ? defaults.fmt : SetVTableInstance(type).fmt; \
        s->toString = (defaults.toString) ? defaults.toString : SetVTableInstance(type).toString; \
        \
        s->put = (defaults.put) ? defaults.put : SetVTableInstance(type).put; \
        s->remove = (defaults.remove) ? defaults.remove : SetVTableInstance(type).remove; \
        \
        s->clear = (defaults.clear) ? defaults.clear : SetVTableInstance(type).clear; \
        s->at = (defaults.at) ? defaults.at : SetVTableInstance(type).at; \
        s->find = (defaults.find) ? defaults.find : SetVTableInstance(type).find; \
        \
        s->copy.set = (defaults.copy.set) ? defaults.copy.set : SetVTableInstance(type).copy.set; \
        s->copy.array = (defaults.copy.array) ? defaults.copy.array : SetVTableInstance(type).copy.array; \
        \
        s->resize = (defaults.resize) ? defaults.resize : SetVTableInstance(type).resize; \
        s->empty = (defaults.empty) ? defaults.empty : SetVTableInstance(type).empty; \
        s->drain = (defaults.drain) ? defaults.drain : SetVTableInstance(type).drain; \
        \
        s->fusion = (defaults.fusion) ? defaults.fusion : SetVTableInstance(type).fusion; \
        s->intersection = (defaults.intersection) ? defaults.intersection : SetVTableInstance(type).intersection; \
        s->sum = (defaults.sum) ? defaults.sum : SetVTableInstance(type).sum; \
        s->diff = (defaults.diff) ? defaults.diff : SetVTableInstance(type).diff; \
        \
        return s; \
    } \

#define Set(type) concat_layer2(Set_, type)
#define newSet(type, cmp, ...) \
    concat_layer2(newSet_, type)(cmp, (struct concat_layer2(_set_defaults_, type)){__VA_ARGS__})

#define forSet_primitive(i, once, vec, acc) \
    for (Iterator i = newIterator((vec)->data, (vec)->size); i && i->size; iterator_advance(&i, _set_next, sizeof(*(vec)->data))) \
        for (acc = (typeof(*(vec)->data) *)i->ref, *once = (void *)1; once; once = 0)

#define forSet(acc, vec) \
    forSet_primitive(concat_layer2(_i_, __COUNTER__), concat_layer2(_once_, __COUNTER__), vec, acc)

#endif

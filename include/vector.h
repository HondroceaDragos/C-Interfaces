#ifndef VECTOR_H
#define VECTOR_H

#include "./array.h"
#include "./vtable_vector.h"
#include "./raise.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define implements(s)

#define VectorType(type) \
    typedef struct concat_layer1(_vector_, type) { \
        type *data; \
        size_t capacity; \
        size_t size; \
        \
        VTableFunctions(struct concat_layer1(_vector_, type) *, type) \
    } *concat_layer1(Vector_, type); \
    \
    VTableType(concat_layer1(Vector_, type), type) \
    \
    struct concat_layer1(_vector_defaults_, type) { \
        size_t capacity; \
        Array(type) using; \
        VTableFunctions(concat_layer1(Vector_, type), type) \
    }; \
    \
    static inline concat_layer1(Vector_, type) concat_layer1(newVector_, type)(struct concat_layer1(_vector_defaults_, type) defaults) { \
        concat_layer1(Vector_, type) v = calloc(1, sizeof(*v)); \
        if (!v) { \
            raise(ERROR, "Cannot create vector " RED "(out-of-memory)" RESET ". Returning nullptr."); \
            return nullptr; \
        } \
        \
        v->capacity = (defaults.capacity < defaults.using.size) ? defaults.using.size : defaults.capacity; \
        if (!v->capacity) v->capacity = VECTOR_DEFAULT_CAPACITY; \
        v->data = calloc(v->capacity, sizeof(type)); \
        if (!v->data) { \
            free(v); \
            raise(ERROR, "Cannot create vector " RED "(out-of-memory)" RESET ". Returning nullptr."); \
            return nullptr; \
        } \
        \
        if (defaults.using.data) { \
            if (defaults.using.size == 1 && v->capacity > 1) { \
                for (size_t idx = 0; idx < v->capacity; idx++) { \
                    v->data[idx] = defaults.using.data[0]; \
                } \
                v->size = v->capacity; \
            } else { \
                memcpy(v->data, defaults.using.data, defaults.using.size * sizeof(type)); \
                v->size = defaults.using.size; \
            } \
        } \
        \
        v->cmp = (defaults.cmp) ? defaults.cmp : VTableInstance(type).cmp; \
        v->hash = (defaults.hash) ? defaults.hash : VTableInstance(type).hash; \
        v->sort = (defaults.sort) ? defaults.sort : VTableInstance(type).sort; \
        \
        v->push = (defaults.push) ? defaults.push : VTableInstance(type).push; \
        v->insert = (defaults.insert) ? defaults.insert : VTableInstance(type).insert; \
        v->pop = (defaults.pop) ? defaults.pop : VTableInstance(type).pop; \
        v->clear = (defaults.clear) ? defaults.clear : VTableInstance(type).clear; \
        v->at = (defaults.at) ? defaults.at : VTableInstance(type).at; \
        v->find = (defaults.find) ? defaults.find : VTableInstance(type).find; \
        v->copy = (defaults.copy) ? defaults.copy : VTableInstance(type).copy; \
        \
        return v; \
    } \

#define Vector(type) concat_layer1(Vector_, type)
#define newVector(type, ...) \
    concat_layer1(newVector_, type)((struct concat_layer1(_vector_defaults_, type)){__VA_ARGS__})

#define forVector_primitive(i, once, value, vec) \
    for (size_t i = 0; i < (vec)->size; i++) \
        for (value = &(vec)->data[i], *once = (void*)1; once; once = nullptr)

#define forVector(value, vec) \
    forVector_primitive(concat_layer2(_i_, __COUNTER__), concat_layer2(_once_, __COUNTER__), value, vec)

#endif

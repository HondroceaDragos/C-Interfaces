#ifndef VECTOR_H
#define VECTOR_H

#include "./array.h"
#include <stdlib.h>
#include <string.h>

#define VECTOR_DEFAULT_CAPACITY 16

typedef int32_t (*CmpFunc)(const void *, const void *);
typedef int32_t (*HashFunc)(const void *);

static inline int32_t _vector_default_cmp(const void *, const void *) { return true; }
static inline int32_t _vector_default_hash(const void *) { return 0; }

#define VectorType(type) \
    typedef struct concat_layer1(_vector_, type) { \
        type *data; \
        size_t capacity; \
        size_t size; \
        \
        CmpFunc cmp; \
        HashFunc hash; \
        void (*sort)(struct concat_layer1(_vector_, type) *); \
        \
        void (*push)(struct concat_layer1(_vector_, type) *, type); \
    } *concat_layer1(Vector_, type); \
    \
    static inline void concat_layer1(_vector_default_sort_, type)(concat_layer1(Vector_, type) self) { \
        qsort(self->data, self->size, sizeof(type), self->cmp); \
    } \
    static inline void concat_layer1(_vector_default_push_, type)(concat_layer1(Vector_, type) self, type elem) { \
        if (self->size == self->capacity) { \
            type *tmp = self->data; \
            tmp = realloc(tmp,  2 * self->capacity * sizeof(type)); \
            if (!tmp) return; \
            self->capacity *= 2; \
            self->data = tmp; \
        } \
        self->data[self->size++] = elem; \
    } \
    struct concat_layer1(_vector_defaults_, type) { \
        size_t capacity; \
        Array(type) using; \
        CmpFunc cmp; \
        HashFunc hash; \
        void (*sort)(concat_layer1(Vector_, type)); \
        void (*push)(concat_layer1(Vector_, type), type); \
    }; \
    \
    static inline concat_layer1(Vector_, type) concat_layer1(newVector_, type)(struct concat_layer1(_vector_defaults_, type) defaults) { \
        concat_layer1(Vector_, type) v = calloc(1, sizeof(*v)); \
        if (!v) return NULL; \
        \
        v->capacity = (defaults.capacity < defaults.using.size) ? defaults.using.size : defaults.capacity; \
        if (!v->capacity) v->capacity = VECTOR_DEFAULT_CAPACITY; \
        v->data = calloc(v->capacity, sizeof(type)); \
        if (!v->data) { free(v); return NULL; } \
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
        v->cmp = (defaults.cmp) ? defaults.cmp : _vector_default_cmp; \
        v->hash = (defaults.hash) ? defaults.hash : _vector_default_hash; \
        v->sort = (defaults.sort) ? defaults.sort : concat_layer1(_vector_default_sort_, type); \
        \
        v->push = (defaults.push) ? defaults.push : concat_layer1(_vector_default_push_, type); \
        \
        return v; \
    } \
    \
    static inline void delete(concat_layer1(Vector_, type))(concat_layer1(Vector_, type) *v) { \
        if (!*v) return; \
        for (size_t idx = 0; idx < (*v)->size; idx++) { \
            delete(type)((*v)->data[idx]); \
        } \
        free((*v)->data); \
        free(*v); \
        *v = NULL; \
    }

#define Vector(type) concat_layer1(Vector_, type)
#define newVector(type, ...) \
    concat_layer1(newVector_, type)((struct concat_layer1(_vector_defaults_, type)){__VA_ARGS__})

#define forVector_primitive(i, once, value, vec) \
    for (size_t i = 0; i < (vec)->size; i++) \
        for (value = &(vec)->data[i], *once = (void*)1; once; once = nullptr)

#define forVector(value, vec) \
    forVector_primitive(concat_layer2(_i_, __COUNTER__), concat_layer2(_once_, __COUNTER__), value, vec)

#endif
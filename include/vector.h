#ifndef VECTOR_H
#define VECTOR_H

#include "./array.h"
#include "./raise.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_DEFAULT_CAPACITY 16
#define notfound -1

typedef int32_t (*CmpFunc)(const void *, const void *);
typedef int32_t (*HashFunc)(const void *);

static inline int32_t _vector_default_cmp(const void *, const void *) { 
    raise(WARNING, "Vector uses " YELLOW "default comparing function " RESET "(always returning \'true\').");
    return true;
}
static inline int32_t _vector_default_hash(const void *) {
    raise(WARNING, "Vector uses " YELLOW "default hashing function " RESET "(always returning \'0\').");
    return 0;
}

#define implements(s)

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
        type (*pop)(struct concat_layer1(_vector_, type) *); \
        void (*clear)(struct concat_layer1(_vector_, type) *); \
        type (*at)(struct concat_layer1(_vector_, type) *, int32_t); \
        int32_t (*find)(struct concat_layer1(_vector_, type) *, type); \
    } *concat_layer1(Vector_, type); \
    \
    static inline void concat_layer1(_vector_default_sort_, type)(concat_layer1(Vector_, type) self) { \
        if (!self->size) { \
            raise(WARNING, "Vector is " YELLOW "empty " RESET "(size = %zu). No sorting to-be-done.", self->size); \
            return; \
        } \
        qsort(self->data, self->size, sizeof(type), self->cmp); \
    } \
    static inline void concat_layer1(_vector_default_clear_, type)(concat_layer1(Vector_, type) self) { \
        self->size = 0; \
    } \
    static inline void concat_layer1(_vector_default_push_, type)(concat_layer1(Vector_, type) self, type elem) { \
        if (self->size == self->capacity) { \
            type *tmp = self->data; \
            tmp = realloc(tmp,  2 * self->capacity * sizeof(type)); \
            if (!tmp) { \
                raise(ERROR, "Cannot expand vector " RED "(out-of-memory)" RESET ". Vector elements will remain unchanged."); \
            } \
            self->capacity *= 2; \
            self->data = tmp; \
        } \
        self->data[self->size++] = elem; \
    } \
    static inline type concat_layer1(_vector_default_pop_, type)(concat_layer1(Vector_, type) self) { \
        if (!self->size) { \
            raise(WARNING, "Vector is " YELLOW "empty " RESET "(size = %zu). Returning 0.", self->size); \
            return (type){0}; \
        } \
        type ret = self->data[--self->size]; \
        return ret; \
    } \
    static inline int32_t concat_layer1(_vector_default_find_, type)(concat_layer1(Vector_, type) self, type dst) { \
        if (self->cmp == _vector_default_cmp) { \
            raise(WARNING, "Vector uses " YELLOW "default comparing function " RESET "(always returning \'true\'). Destination not found."); \
            return notfound; \
        } \
        for (size_t idx = 0; idx < self->size; idx++) { \
            if (self->cmp(&(self->data[idx]), &dst) == 0) return idx; \
        } \
        \
        return notfound; \
    } \
    static inline type concat_layer1(_vector_default_at_, type)(concat_layer1(Vector_, type) self, int32_t idx) { \
        int32_t size = (int32_t)self->size; \
        if (!size) { \
            raise(WARNING, "Vector is " YELLOW "empty " RESET "(size = %zu). Returning 0.", self->size); \
            return (type){0}; \
        } \
        if (idx >= size) { \
            raise(WARNING, "Index " YELLOW "out-of-bounds" RESET ". idx = %d is not in range [0, %d]. Returning 0.", idx, size); \
            return (type){0}; \
        } \
        if (idx < -size) { \
            raise(WARNING, "Index " YELLOW "out-of-bounds" RESET ". idx = %d is not in range [%d, -1] Returning 0.", idx, -size); \
            return (type){0}; \
        } \
        idx = (idx < 0) ? (size + idx) : idx; \
        return self->data[idx]; \
    } \
    struct concat_layer1(_vector_defaults_, type) { \
        size_t capacity; \
        Array(type) using; \
        CmpFunc cmp; \
        HashFunc hash; \
        void (*sort)(concat_layer1(Vector_, type)); \
        void (*push)(concat_layer1(Vector_, type), type); \
        type (*pop)(concat_layer1(Vector_, type)); \
        void (*clear)(concat_layer1(Vector_, type)); \
        type (*at)(concat_layer1(Vector_, type), int32_t); \
        int32_t (*find)(concat_layer1(Vector_, type), type); \
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
        v->cmp = (defaults.cmp) ? defaults.cmp : _vector_default_cmp; \
        v->hash = (defaults.hash) ? defaults.hash : _vector_default_hash; \
        v->sort = (defaults.sort) ? defaults.sort : concat_layer1(_vector_default_sort_, type); \
        \
        v->push = (defaults.push) ? defaults.push : concat_layer1(_vector_default_push_, type); \
        v->pop = (defaults.pop) ? defaults.pop : concat_layer1(_vector_default_pop_, type); \
        v->clear = (defaults.clear) ? defaults.clear : concat_layer1(_vector_default_clear_, type); \
        v->at = (defaults.at) ? defaults.at : concat_layer1(_vector_default_at_, type); \
        v->find = (defaults.find) ? defaults.find : concat_layer1(_vector_default_find_, type); \
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

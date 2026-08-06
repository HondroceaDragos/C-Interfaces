#ifndef VTABLE_VECTOR_H
#define VTABLE_VECTOR_H

#include "./helpers.h"
#include "./raise.h"
#include <stdlib.h>
#include <stdint.h>
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

typedef enum : int8_t {
    RETAIN_MEMORY,
    FREE_MEMORY
} MemoryCleanup;

#define VTableFunctions(id, type) \
    CmpFunc cmp; \
    HashFunc hash; \
    void (*sort)(id); \
    void (*push)(id, type); \
    void (*insert)(id, type, int32_t); \
    type (*pop)(id); \
    void (*clear)(id, MemoryCleanup); \
    type (*at)(id, int32_t); \
    int32_t (*find)(id, type); \
    void (*copy)(id, id, MemoryCleanup);

#define VTableType(id, type) \
    typedef struct concat_layer1(_vtable_, type) { \
        VTableFunctions(id, type) \
    } concat_layer1(VTable_, type); \
    static inline void concat_layer1(_vector_default_sort_, type)(id self) { \
        if (!self->size) { \
            raise(WARNING, "Vector is " YELLOW "empty " RESET "(size = %zu). No sorting to-be-done.", self->size); \
            return; \
        } \
        qsort(self->data, self->size, sizeof(type), self->cmp); \
    } \
    static inline void delete(id)(id *v) { \
        if (!v || !*v) return; \
        for (size_t idx = 0; idx < (*v)->size; idx++) { \
            delete(type)(&(*v)->data[idx]); \
        } \
        free((*v)->data); \
        free(*v); \
        *v = NULL; \
    } \
    static inline void concat_layer1(_vector_default_clear_, type)(id self, MemoryCleanup mc) { \
        if (mc == FREE_MEMORY) { \
            for (size_t idx = 0; idx < self->size; idx++) { \
                delete(type)(&(self->data[idx])); \
            } \
        } \
        self->size = 0; \
    } \
    static inline void concat_layer1(_vector_default_push_, type)(id self, type elem) { \
        if (self->size == self->capacity) { \
            type *tmp = self->data; \
            tmp = realloc(self->data, 2 * self->capacity * sizeof(type)); \
            if (!tmp) { \
                raise(ERROR, "Cannot expand vector " RED "(out-of-memory)" RESET ". Vector elements will remain unchanged."); \
            } \
            self->capacity *= 2; \
            self->data = tmp; \
        } \
        self->data[self->size++] = elem; \
    } \
    static inline void concat_layer1(_vector_default_insert_, type)(id self, type elem, int32_t idx) { \
        int32_t size = (int32_t)self->size; \
        idx = (idx < 0) ? (size + idx + 1) : idx; \
        \
        size_t new_size = (idx < 0) ? (self->size + (size_t)(-idx) + 1) : (((size_t)idx <= self->size) ? (self->size + 1) : ((size_t)idx + 1)); \
        size_t new_cap = (self->capacity) ? self->capacity : VECTOR_DEFAULT_CAPACITY; \
        \
        if (new_size > self->capacity) { \
            while (new_size > new_cap) new_cap *= 2; \
            type *tmp = self->data; \
            tmp = realloc(self->data, new_cap * sizeof(type)); \
            if (!tmp) { \
                raise(ERROR, "Cannot expand vector " RED "(out-of-memory)" RESET ". Vector elements will remain unchanged."); \
            } \
            self->capacity = new_cap; \
            self->data = tmp; \
        } \
        \
        if (idx < 0) { \
            memmove(self->data + (size_t)(-idx) + 1, self->data, self->size * sizeof(type)); \
            memset(self->data + 1, 0, (size_t)(-idx) * sizeof(type)); \
            idx = 0; \
        } else if ((size_t)idx <= self->size) { \
            memmove(self->data + (size_t)idx + 1, self->data + (size_t)idx, (self->size - (size_t)idx) * sizeof(type)); \
        } else { \
            memset(self->data + self->size, 0, ((size_t)idx - self->size) * sizeof(type)); \
        } \
        \
        self->size = new_size; \
        self->data[idx] = elem; \
    } \
    static inline type concat_layer1(_vector_default_pop_, type)(id self) { \
        if (!self->size) { \
            raise(WARNING, "Vector is " YELLOW "empty " RESET "(size = %zu). Returning 0.", self->size); \
            return (type){0}; \
        } \
        type ret = self->data[--self->size]; \
        return ret; \
    } \
    static inline int32_t concat_layer1(_vector_default_find_, type)(id self, type dst) { \
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
    static inline type concat_layer1(_vector_default_at_, type)(id self, int32_t idx) { \
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
    static inline void concat_layer1(_vector_default_copy_, type)(id self, id other, MemoryCleanup mc) { \
        if (mc == FREE_MEMORY) { \
            for (size_t idx = 0; idx < self->size; idx++) { \
                delete(type)(&(self->data[idx])); \
            } \
        } \
        if (other->size > self->size) { \
            if (self->capacity < other->size) { \
                size_t new_cap = self->capacity; \
                while (new_cap < other->size) new_cap *= 2; \
                type *tmp =  self->data; \
                tmp = realloc(self->data, new_cap * sizeof(type)); \
                if (!tmp) return; \
                self->data = tmp; \
                self->capacity = new_cap; \
            } \
        } \
        self->size = other->size; \
        memcpy(self->data, other->data, self->size * sizeof(*self->data)); \
    } \
    static concat_layer1(VTable_, type) concat_layer2(VTable_, concat_layer2(vector_, type)) = { \
        .cmp = _vector_default_cmp, \
        .hash = _vector_default_hash, \
        .sort = concat_layer1(_vector_default_sort_, type), \
        .push = concat_layer1(_vector_default_push_, type), \
        .insert = concat_layer1(_vector_default_insert_, type), \
        .pop = concat_layer1(_vector_default_pop_, type), \
        .clear = concat_layer1(_vector_default_clear_, type), \
        .at = concat_layer1(_vector_default_at_, type), \
        .find = concat_layer1(_vector_default_find_, type), \
        .copy = concat_layer1(_vector_default_copy_, type) \
    }; \

#define VTable(type) concat_layer1(VTable_, type)
#define VTableInstance(type) concat_layer2(VTable_, concat_layer2(vector_, type))

#endif

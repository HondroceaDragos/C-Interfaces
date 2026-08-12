#ifndef VTABLE_VECTOR_H
#define VTABLE_VECTOR_H

#include "./helpers.h"
#include "./raise.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
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

#define VectorVTableFunctions(id, type) \
    CmpFunc cmp; \
    HashFunc hash; \
    int8_t *(*fmt)(const type); \
    void (*sort)(id); \
    void (*push)(id, type); \
    void (*insert)(id, type, int32_t); \
    type (*remove)(id, int32_t); \
    type (*pop)(id); \
    void (*clear)(id, MemoryCleanup); \
    type (*at)(id, int32_t); \
    int32_t (*find)(id, type); \
    void (*copy)(id, id, MemoryCleanup); \
    int8_t *(*toString)(id); \
    void (*resize)(id, size_t, type, MemoryCleanup); \
    bool (*empty)(id); \
    struct { \
        void (*vector)(id, id); \
        void (*array)(id, concat_layer2(Array_, type)); \
    } multiPush; \
    type *(*drain)(id, size_t); \

#define VectorVTableType(id, type) \
    typedef struct concat_layer2(_vector_vtable_, type) { \
        VectorVTableFunctions(id, type) \
    } concat_layer2(VectorVTable_, type); \
    static inline void concat_layer2(_vector_default_sort_, type)(id self) { \
        if (!self->size) { \
            raise(WARNING, "Vector is " YELLOW "empty " RESET "(size = %zu). No sorting to-be-done.", self->size); \
            return; \
        } \
        qsort(self->data, self->size, sizeof(type), self->cmp); \
    } \
    static inline int8_t *concat_layer2(_vector_default_fmt_, type)(const type) { \
        raise(WARNING, "Vector uses " YELLOW "default formatting function " RESET "(cannot deduce type)."); \
        return strdup("[?]"); \
    } \
    static inline void delete(id)(id *v) { \
        if (!v || !*v) return; \
        for (size_t idx = 0; idx < (*v)->size; idx++) { \
            delete(type)(&(*v)->data[idx]); \
        } \
        free((*v)->data); \
        free(*v); \
        *v = nullptr; \
    } \
    static inline void concat_layer2(_vector_default_clear_, type)(id self, MemoryCleanup mc) { \
        if (mc == FREE_MEMORY) { \
            for (size_t idx = 0; idx < self->size; idx++) { \
                delete(type)(&(self->data[idx])); \
            } \
        } \
        self->size = 0; \
    } \
    static inline void concat_layer2(_vector_default_push_, type)(id self, type elem) { \
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
    static inline void concat_layer2(_vector_default_insert_, type)(id self, type elem, int32_t idx) { \
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
    static inline type concat_layer2(_vector_default_remove_, type)(id self, int32_t idx) { \
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
        \
        type ret = self->data[idx]; \
        memmove(self->data + idx, self->data + idx + 1, (size_t)(size - idx - 1) * sizeof(*self->data)); \
        self->size--; \
        return ret; \
    } \
    static inline type concat_layer2(_vector_default_pop_, type)(id self) { \
        if (!self->size) { \
            raise(WARNING, "Vector is " YELLOW "empty " RESET "(size = %zu). Returning 0.", self->size); \
            return (type){0}; \
        } \
        type ret = self->data[--self->size]; \
        return ret; \
    } \
    static inline int32_t concat_layer2(_vector_default_find_, type)(id self, type dst) { \
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
    static inline type concat_layer2(_vector_default_at_, type)(id self, int32_t idx) { \
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
    static inline void concat_layer2(_vector_default_copy_, type)(id self, id other, MemoryCleanup mc) { \
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
                if (!tmp) { \
                    raise(ERROR, "Cannot expand vector " RED "(out-of-memory)" RESET ". Vector elements will remain unchanged."); \
                } \
                self->data = tmp; \
                self->capacity = new_cap; \
            } \
        } \
        self->size = other->size; \
        memcpy(self->data, other->data, self->size * sizeof(*self->data)); \
    } \
    static inline int8_t *concat_layer2(_vector_default_tostring_, type)(id self) { \
        size_t bsize = 0; \
        size_t bcap = 2048; \
        \
        int8_t *buffer = calloc(bcap, sizeof(*buffer)); \
        buffer[0] = '\0'; \
        \
        int32_t slen = snprintf(buffer, bcap, "Vector(%zu <= %zu) {\n\t", self->size, self->capacity); \
        bsize += slen; \
        size_t size = self->size; \
        \
        for (size_t idx = 0; idx < size; idx++) { \
            int8_t *element = self->fmt(self->data[idx]); \
            size_t elen = strlen(element); \
            \
            const int8_t *sep = (idx + 1 < size) ? ", " : "\n"; \
            size_t seplen = strlen(sep); \
            \
            size_t space = bsize + elen + seplen + 1; \
            \
            if (space > bcap) { \
                size_t new_cap = bcap; \
                while (space > new_cap) new_cap *= 2; \
                int8_t *tmp = buffer; \
                tmp = realloc(buffer, new_cap * sizeof(*buffer)); \
                if (!tmp) { \
                    raise(ERROR, "Cannot build vector string " RED "(out-of-memory)" RESET ". Vector elements will remain unchanged."); \
                    free(element); \
                    free(buffer); \
                    return strdup("[?]"); \
                } \
                buffer = tmp; \
                bcap = new_cap; \
            } \
            \
            bsize += snprintf(buffer + bsize, bcap - bsize, "%s%s", element, sep); \
            free(element); \
        } \
        \
        snprintf(buffer + bsize, bcap - bsize, "}"); \
        return buffer; \
    } \
    static inline void concat_layer2(_vector_default_resize_, type)(id self, size_t size, type element, MemoryCleanup mc) { \
        if (mc == FREE_MEMORY) { \
            for (size_t idx = 0; idx < self->size; idx++) { \
                delete(type)(&(self->data[idx])); \
            } \
        } \
        \
        type *tmp = self->data; \
        tmp = realloc(self->data, size * sizeof(type)); \
        if (!tmp) { \
            raise(ERROR, "Cannot resize vector " RED "(out-of-memory)" RESET ". Vector elements will remain unchanged."); \
        } \
        self->capacity = size; \
        self->data = tmp; \
        \
        self->size = self->capacity; \
        for (size_t idx = 0; idx < self->size; idx++) { \
            self->data[idx] = element; \
        } \
    } \
    static inline bool concat_layer2(_vector_default_empty_, type)(id self) { \
        if (self->size == 0) return true; \
        return false; \
    } \
    static inline void concat_layer2(_vector_default_multipush_vector_, type)(id self, id other) { \
        if (self == other) { \
            raise(ERROR, "Cannot " RED "push " RESET "a "  RED "self-reference" RESET "."); \
        } \
        if (self->size + other->size >= self->capacity) { \
            size_t new_cap = self->capacity; \
            while (new_cap < self->size + other->size) new_cap *= 2; \
            type *tmp = self->data; \
            tmp = realloc(self->data, new_cap * sizeof(type)); \
            if (!tmp) { \
                raise(ERROR, "Cannot expand vector " RED "(out-of-memory)" RESET ". Vector elements will remain unchanged."); \
            } \
            self->capacity = new_cap; \
            self->data = tmp; \
        } \
        memcpy(self->data + self->size, other->data, other->size * sizeof(*self->data)); \
        self->size += other->size; \
    } \
    static inline void concat_layer2(_vector_default_multipush_array_, type)(id self, concat_layer2(Array_, type) other) { \
        if (self->size + other.size >= self->capacity) { \
            size_t new_cap = self->capacity; \
            while (new_cap < self->size + other.size) new_cap *= 2; \
            type *tmp = self->data; \
            tmp = realloc(self->data, new_cap * sizeof(type)); \
            if (!tmp) { \
                raise(ERROR, "Cannot expand vector " RED "(out-of-memory)" RESET ". Vector elements will remain unchanged."); \
            } \
            self->capacity = new_cap; \
            self->data = tmp; \
        } \
        memcpy(self->data + self->size, other.data, other.size * sizeof(*self->data)); \
        self->size += other.size; \
    } \
    static inline type *concat_layer2(_vector_default_drain_, type)(id self, size_t many) { \
        if (!self->size) { \
            raise(WARNING, "Vector is " YELLOW "empty " RESET "(size = %zu). Returning nullptr.", self->size); \
            return nullptr; \
        } \
        if (self->size < many) { \
            raise(WARNING, "Too many elements " YELLOW "to-be-popped " RESET "(size = %zu < many = %zu). Returning the last %zu elements from vector.", self->size, many, self->size); \
            many = self->size; \
        } \
        type *ret = calloc(many, sizeof(*ret)); \
        if (!ret) { \
            raise(ERROR, "Memory request denied " RED "(out-of-memory)" RESET ". Vector elements will remain unchanged."); \
        } \
        memcpy(ret, self->data + (self->size - many), many * sizeof(*ret)); \
        self->size -= many; \
        \
        return ret; \
    } \
    static concat_layer2(VectorVTable_, type) concat_layer2(VectorVTable_, concat_layer2(vector_, type)) = { \
        .cmp = _vector_default_cmp, \
        .hash = _vector_default_hash, \
        .fmt = concat_layer2(_vector_default_fmt_, type), \
        .sort = concat_layer2(_vector_default_sort_, type), \
        .push = concat_layer2(_vector_default_push_, type), \
        .insert = concat_layer2(_vector_default_insert_, type), \
        .remove = concat_layer2(_vector_default_remove_, type), \
        .pop = concat_layer2(_vector_default_pop_, type), \
        .clear = concat_layer2(_vector_default_clear_, type), \
        .at = concat_layer2(_vector_default_at_, type), \
        .find = concat_layer2(_vector_default_find_, type), \
        .copy = concat_layer2(_vector_default_copy_, type), \
        .toString = concat_layer2(_vector_default_tostring_, type), \
        .resize = concat_layer2(_vector_default_resize_, type), \
        .empty = concat_layer2(_vector_default_empty_, type), \
        .multiPush.vector = concat_layer2(_vector_default_multipush_vector_, type), \
        .multiPush.array = concat_layer2(_vector_default_multipush_array_, type), \
        .drain = concat_layer2(_vector_default_drain_, type) \
    }; \

#define VectorVTable(type) concat_layer2(VectorVTable_, type)
#define VectorVTableInstance(type) concat_layer2(VectorVTable_, concat_layer2(vector_, type))

#endif

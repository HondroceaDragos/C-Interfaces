#ifndef VTABLE_SET_H
#define VTABLE_SET_H

#include "./helpers.h"
#include "./raise.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#define SET_DEFAULT_CAPACITY 16

static inline int32_t _set_default_cmp(const void *, const void *) { 
    raise(WARNING, "Set uses " YELLOW "default comparing function " RESET "(always returning \'true\').");
    return true;
}

#define SetVTableFunctions(id, type) \
    bool (*filter)(type); \
    int8_t *(*fmt)(const type); \
    int8_t *(*toString)(id); \
    void (*put)(id, type); \
    type (*remove)(id, type); \
    void (*clear)(id, MemoryCleanup); \
    type (*at)(id, int32_t); \
    int32_t (*find)(id, type); \
    struct { \
        void (*set)(id, id, MemoryCleanup); \
        void (*array)(id, concat_layer2(Array_, type), MemoryCleanup); \
    } copy; \
    void (*resize)(id, size_t, MemoryCleanup); \
    bool (*empty)(id); \
    type *(*drain)(id, size_t); \
    id (*fusion)(id, id); \
    id (*intersection)(id, id); \
    id (*sum)(id, id); \
    id (*diff)(id, id); \


#define SetVTableType(id, type) \
    typedef struct concat_layer2(_set_vtable_, type) { \
        SetVTableFunctions(id, type) \
    } concat_layer2(SetVTable_, type); \
    static inline int8_t *concat_layer2(_set_default_fmt_, type)(const type) { \
        raise(WARNING, "Set uses " YELLOW "default formatting function " RESET "(cannot deduce type)."); \
        return strdup("[?]"); \
    } \
    static inline void delete(id)(id *s) { \
        if (!s || !*s) return; \
        for (size_t idx = 0; idx < (*s)->size; idx++) { \
            delete(type)(&(*s)->data[idx]); \
        } \
        free((*s)->data); \
        free(*s); \
        *s = nullptr; \
    } \
    static inline void concat_layer2(_set_default_clear_, type)(id self, MemoryCleanup mc) { \
        if (mc == FREE_MEMORY) { \
            for (size_t idx = 0; idx < self->size; idx++) { \
                delete(type)(&(self->data[idx])); \
            } \
        } \
        self->size = 0; \
    } \
    static inline void concat_layer2(_set_default_put_, type)(id self, type elem) { \
        if (self->filter) if (!self->filter(elem)) return; \
        if (bsearch(&elem, self->data, self->size, sizeof(type), self->cmp)) return; \
        \
        if (self->size == self->capacity) { \
            type *tmp = self->data; \
            tmp = realloc(self->data, 2 * self->capacity * sizeof(type)); \
            if (!tmp) { \
                raise(ERROR, "Cannot expand set " RED "(out-of-memory)" RESET ". Set elements will remain unchanged."); \
            } \
            self->capacity *= 2; \
            self->data = tmp; \
        } \
        \
        size_t left = 0; \
        size_t right = self->size; \
        while (left < right) { \
            size_t mid = (left + right) / 2; \
            \
            if (self->cmp(&self->data[mid], &elem) < 0) left = mid + 1; \
            else right = mid; \
        } \
        \
        memmove(self->data + left + 1, self->data + left, (self->size - left) * sizeof(type)); \
        self->data[left] = elem; \
        self->size++; \
    } \
    static inline type concat_layer2(_set_default_remove_, type)(id self, type target) { \
        type *ret = (type *)bsearch(&target, self->data, self->size, sizeof(type), self->cmp); \
        if (!ret) { \
            if (self->fmt) { \
                int8_t *warr = self->fmt(target); \
                raise(WARNING, "Element " YELLOW "not in set" RESET ": %s. Set elements will remain unchanged.", warr); \
                raise(WARNING, "Returning zero."); \
                free(warr); \
            } else { \
                raise(WARNING, "Element " YELLOW "not in set" RESET ". Set elements will remain unchanged."); \
                raise(WARNING, "Returning zero."); \
            } \
            return (type){0}; \
        } \
        \
        size_t idx = ret - self->data; \
        type elem = self->data[idx]; \
        memmove(ret, ret + 1, (self->size - idx - 1) * sizeof(type)); \
        self->size--; \
        \
        return elem; \
    } \
    static inline int32_t concat_layer2(_set_default_find_, type)(id self, type dst) { \
        type *item = (type *)bsearch(&dst, self->data, self->size, sizeof(type), self->cmp); \
        if (!item) return notfound; \
        return item - self->data; \
    } \
    static inline type concat_layer2(_set_default_at_, type)(id self, int32_t idx) { \
        int32_t size = (int32_t)self->size; \
        if (!size) { \
            raise(WARNING, "Set is " YELLOW "empty " RESET "(size = %zu). Returning 0.", self->size); \
            return (type){0}; \
        } \
        if (idx >= size) { \
            raise(WARNING, "Index " YELLOW "out-of-bounds" RESET ". idx = %d is not in range [0, %d]. Returning 0.", idx, size); \
            return (type){0}; \
        } \
        if (idx < -size) { \
            raise(WARNING, "Index " YELLOW "out-of-bounds" RESET ". idx = %d is not in range [%d, -1]. Returning 0.", idx, -size); \
            return (type){0}; \
        } \
        idx = (idx < 0) ? (size + idx) : idx; \
        return self->data[idx]; \
    } \
    static inline void concat_layer2(_set_default_copy_set_, type)(id self, id other, MemoryCleanup mc) { \
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
                    raise(ERROR, "Cannot expand set " RED "(out-of-memory)" RESET ". Set elements will remain unchanged."); \
                } \
                self->data = tmp; \
                self->capacity = new_cap; \
            } \
        } \
        \
        size_t ulen = 0; \
        for (size_t idx = 0; idx < other->size; idx++) { \
            if (self->filter) if (self->filter(other->data[idx])) self->data[ulen++] = other->data[idx]; \
        } \
        self->size = ulen; \
    } \
    static inline void concat_layer2(_set_default_copy_array_, type)(id self, concat_layer2(Array_, type) other, MemoryCleanup mc) { \
        if (mc == FREE_MEMORY) { \
            for (size_t idx = 0; idx < self->size; idx++) { \
                delete(type)(&(self->data[idx])); \
            } \
        } \
        if (other.size > self->size) { \
            if (self->capacity < other.size) { \
                size_t new_cap = self->capacity; \
                while (new_cap < other.size) new_cap *= 2; \
                type *tmp =  self->data; \
                tmp = realloc(self->data, new_cap * sizeof(type)); \
                if (!tmp) { \
                    raise(ERROR, "Cannot expand set " RED "(out-of-memory)" RESET ". Set elements will remain unchanged."); \
                } \
                self->data = tmp; \
                self->capacity = new_cap; \
            } \
        } \
        \
        memcpy(self->data, other.data, other.size * sizeof(type)); \
        self->size = other.size; \
        qsort(self->data, self->size, sizeof(type), self->cmp); \
        \
        size_t ulen = 0; \
        for (size_t idx = 0; idx < self->size; idx++) { \
            if (self->filter) if (!self->filter(self->data[idx])) continue; \
            if (self->cmp(&self->data[idx], &self->data[idx + 1])) self->data[ulen++] = self->data[idx]; \
        } \
        self->size = ulen; \
    } \
    static inline int8_t *concat_layer2(_set_default_tostring_, type)(id self) { \
        size_t bsize = 0; \
        size_t bcap = 512; \
        \
        int8_t *buffer = calloc(bcap, sizeof(*buffer)); \
        buffer[0] = '\0'; \
        \
        int32_t slen = snprintf(buffer, bcap, "Set(%zu <= %zu) {\n\t", self->size, self->capacity); \
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
                    raise(ERROR, "Cannot build set string " RED "(out-of-memory)" RESET ". Set elements will remain unchanged."); \
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
    static inline void concat_layer2(_set_default_resize_, type)(id self, size_t size, MemoryCleanup mc) { \
        if (mc == FREE_MEMORY) { \
            for (size_t idx = 0; idx < self->size; idx++) { \
                delete(type)(&(self->data[idx])); \
            } \
        } \
        \
        type *tmp = self->data; \
        tmp = realloc(self->data, size * sizeof(type)); \
        if (!tmp) { \
            raise(ERROR, "Cannot resize set " RED "(out-of-memory)" RESET ". Set elements will remain unchanged."); \
        } \
        self->capacity = size; \
        self->data = tmp; \
        \
        self->size = self->capacity; \
    } \
    static inline bool concat_layer2(_set_default_empty_, type)(id self) { \
        return (self->size == 0); \
    } \
    static inline type *concat_layer2(_set_default_drain_, type)(id self, size_t many) { \
        if (!self->size) { \
            raise(WARNING, "Set is " YELLOW "empty " RESET "(size = %zu). Returning nullptr.", self->size); \
            return nullptr; \
        } \
        if (self->size < many) { \
            raise(WARNING, "Too many elements " YELLOW "to-be-popped " RESET "(size = %zu < many = %zu). Returning the last %zu elements from set.", self->size, many, self->size); \
            many = self->size; \
        } \
        type *ret = calloc(many, sizeof(*ret)); \
        if (!ret) { \
            raise(ERROR, "Memory request denied " RED "(out-of-memory)" RESET ". Set elements will remain unchanged."); \
        } \
        memcpy(ret, self->data + (self->size - many), many * sizeof(*ret)); \
        self->size -= many; \
        \
        return ret; \
    } \
    static inline id concat_layer2(_set_default_union_, type)(id self, id other) { \
        id ret = calloc(1, sizeof(*ret)); \
        if (!ret) { \
            raise(ERROR, "Cannot " RED "create set " RESET "(out-of-memory). Returning nullptr."); \
            return nullptr; \
        } \
        *ret = *self; \
        ret->capacity = self->size + other->size; \
        \
        ret->data = calloc(ret->capacity, sizeof(type)); \
        if (!ret->data) { \
            free(ret); \
            raise(ERROR, "Cannot " RED "create set " RESET "(out-of-memory)."); \
            return nullptr; \
        } \
        \
        ret->size = 0; \
        \
        for (size_t idx = 0; idx < self->size; idx++) ret->put(ret, self->data[idx]); \
        for (size_t idx = 0; idx < other->size; idx++) ret->put(ret, other->data[idx]); \
        \
        return ret; \
    } \
    static inline id concat_layer2(_set_default_intersection_, type)(id self, id other) { \
        id smaller = self; \
        id bigger = other; \
        size_t minlen = self->capacity; \
        if (self->capacity > other->capacity) { \
            smaller = other; \
            minlen = other->size; \
            bigger = self; \
        } \
        \
        id ret = calloc(1, sizeof(*ret)); \
        if (!ret) { \
            raise(ERROR, "Cannot " RED "create set " RESET "(out-of-memory). Returning nullptr."); \
            return nullptr; \
        } \
        *ret = *self; \
        ret->capacity = minlen; \
        \
        ret->data = calloc(ret->capacity, sizeof(type)); \
        if (!ret->data) { \
            free(ret); \
            raise(ERROR, "Cannot " RED "create set " RESET "(out-of-memory)."); \
            return nullptr; \
        } \
        \
        ret->size = 0; \
        \
        for (size_t idx = 0; idx < minlen; idx++) { \
            type elem = smaller->data[idx]; \
            if (bsearch(&elem, bigger->data, bigger->size, sizeof(type), smaller->cmp)) ret->put(ret, elem); \
        } \
        \
        return ret; \
    } \
    static inline id concat_layer2(_set_default_sum_, type)(id self, id other) { \
        id ret = calloc(1, sizeof(*ret)); \
        if (!ret) { \
            raise(ERROR, "Cannot " RED "create set " RESET "(out-of-memory). Returning nullptr."); \
            return nullptr; \
        } \
        *ret = *self; \
        ret->capacity = self->size + other->size; \
        \
        ret->data = calloc(ret->capacity, sizeof(type)); \
        if (!ret->data) { \
            free(ret); \
            raise(ERROR, "Cannot " RED "create set " RESET "(out-of-memory)."); \
            return nullptr; \
        } \
        \
        ret->size = 0; \
        ret->filter = nullptr; \
        \
        for (size_t idx = 0; idx < self->size; idx++) ret->put(ret, self->data[idx]); \
        for (size_t idx = 0; idx < other->size; idx++) ret->put(ret, other->data[idx]); \
        \
        return ret; \
    } \
    static inline id concat_layer2(_set_default_diff_, type)(id self, id other) { \
        id ret = calloc(1, sizeof(*ret)); \
        if (!ret) { \
            raise(ERROR, "Cannot " RED "create set " RESET "(out-of-memory). Returning nullptr."); \
            return nullptr; \
        } \
        *ret = *self; \
        ret->capacity = self->size; \
        \
        ret->data = calloc(ret->capacity, sizeof(type)); \
        if (!ret->data) { \
            free(ret); \
            raise(ERROR, "Cannot " RED "create set " RESET "(out-of-memory)."); \
            return nullptr; \
        } \
        \
        ret->size = 0; \
        \
        for (size_t idx = 0; idx < self->size; idx++) { \
            type elem = self->data[idx]; \
            if (!bsearch(&elem, other->data, other->size, sizeof(type), self->cmp)) ret->put(ret, elem); \
        } \
        \
        return ret; \
    } \
    static concat_layer2(SetVTable_, type) concat_layer2(SetVTable_, concat_layer2(set_, type)) = { \
        .fmt = concat_layer2(_set_default_fmt_, type), \
        .put = concat_layer2(_set_default_put_, type), \
        .remove = concat_layer2(_set_default_remove_, type), \
        .filter = nullptr, \
        .clear = concat_layer2(_set_default_clear_, type), \
        .at = concat_layer2(_set_default_at_, type), \
        .find = concat_layer2(_set_default_find_, type), \
        .copy.set = concat_layer2(_set_default_copy_set_, type), \
        .copy.array = concat_layer2(_set_default_copy_array_, type), \
        .toString = concat_layer2(_set_default_tostring_, type), \
        .resize = concat_layer2(_set_default_resize_, type), \
        .empty = concat_layer2(_set_default_empty_, type), \
        .drain = concat_layer2(_set_default_drain_, type), \
        .fusion = concat_layer2(_set_default_union_, type), \
        .intersection = concat_layer2(_set_default_intersection_, type), \
        .sum = concat_layer2(_set_default_sum_, type), \
        .diff = concat_layer2(_set_default_diff_, type), \
    }; \

#define SetVTable(type) concat_layer2(SetVTable_, type)
#define SetVTableInstance(type) concat_layer2(SetVTable_, concat_layer2(set_, type))

#endif

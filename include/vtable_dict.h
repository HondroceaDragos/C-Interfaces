#ifndef VTABLE_DICT_H
#define VTABLE_DICT_H

#include "./helpers.h"
#include "./raise.h"
#include "./node.h"
#include "./pair.h"
#include "./stdtypes.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

typedef const int8_t *DictKey;

deleteDefine(DictKey) {
    if (!self || !*self) return;
    free((void *)*self);
}

static inline const ui64 dict_default_hash(DictKey data, size_t dataSize, size_t dictCap) {
    size_t seed = 5381;
    for (size_t idx = 0; idx < dataSize; idx++) {
        seed = ((seed << 5) + seed) + data[idx];
    }
    return seed % dictCap;
}

#define DictVTableFunctions(id, type) \
    const ui64 (*hash)(DictKey, size_t, size_t); \
    i8 *(*fmt)(DictKey, const type); \
    i8 *(*toString)(id); \
    void (*put)(id, Pair(DictKey, type)); \
    void (*emplace)(id, const i8 *, type); \
    type (*get)(id, const i8 *); \
    f64 (*currLoad)(id); \
    type (*remove)(id, const i8 *); \
    bool (*contains)(id, const i8 *);

#define DictVTableType(id, type) \
    typedef struct concat_layer2(_dict_vtable_, type) { \
        DictVTableFunctions(id, type) \
    } concat_layer2(DictVTable_, type); \
    \
    static inline i8 *concat_layer2(_dict_default_fmt_, type)(DictKey, const type) { \
        raise(WARNING, "Dict uses " YELLOW "default formatting function " RESET "(cannot deduce type)."); \
        return strdup("[?]"); \
    } \
    static inline i8 *concat_layer2(_dict_default_toString_, type)(id self) { \
        if (!self) raise(ERROR, "Cannot format an " RED "empty container " RESET "(nil)."); \
        \
        size_t bsize = 0; \
        size_t bcap = 256; \
        \
        int8_t *buffer = calloc(bcap, sizeof(*buffer)); \
        buffer[0] = '\0'; \
        \
        double currLoad = (1.0 * self->size) / self->capacity; \
        int32_t slen = snprintf(buffer, bcap, "Dict(%zu / %zu = %g <= %g) {\n", \
            self->size, self->capacity, currLoad, self->loadFactor); \
        bsize += slen; \
        size_t size = self->capacity; \
        \
        for (size_t idx = 0; idx < size; idx++) { \
            Node(Pair(DictKey, type)) content = self->data[idx]; \
            if (!content) continue; \
            \
            bsize += snprintf(buffer + bsize, bcap - bsize, "[%zu]:\n\t", idx); \
            \
            while (content) { \
                i8 *element = self->fmt(content->value.first, content->value.second); \
                if (!element) raise(ERROR, "Out-of-Memory"); \
                size_t elen = strlen(element); \
                \
                Node(Pair(DictKey, type)) next = content->link.next ? getNode(Pair(DictKey, type), content->link.next) : nullptr; \
                \
                const i8 *sep = (next) ? " <->\n\t" : "\n"; \
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
                        raise(ERROR, "Cannot build dict string " RED "(out-of-memory)" RESET ". Dict elements will remain unchanged."); \
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
                content = next; \
            } \
        } \
        snprintf(buffer + bsize, bcap - bsize, "}"); \
        \
        return buffer; \
    } \
    \
    static inline void delete(id)(id *d) { \
        if (!d || !*d) return; \
        for (size_t idx = 0; idx < (*d)->capacity; idx++) { \
            Node(Pair(DictKey, type)) iter = (*d)->data[idx]; \
            while (iter) { \
                Node(Pair(DictKey, type)) next = iter->link.next ? getNode(Pair(DictKey, type), iter->link.next) : nullptr; \
                free((void *)iter->value.first); \
                delete(Node(Pair(DictKey, type)))(&iter); \
                iter = next; \
            } \
        } \
        free((*d)->data); \
        free(*d); \
        *d = nullptr; \
    } \
    static inline void concat_layer2(dictRehash_, type)(id self) { \
        size_t new_cap = 2 * self->capacity; \
        Node(Pair(DictKey, type)) *tmp = calloc(new_cap, sizeof(*tmp)); \
        \
        for (size_t idx = 0; idx < self->capacity; idx++) { \
            Node(Pair(DictKey, type)) iter = self->data[idx]; \
            while (iter) { \
                Node(Pair(DictKey, type)) next = getNode(Pair(DictKey, type), iter->link.next); \
                size_t new_idx = self->hash(iter->value.first, strlen(iter->value.first), new_cap); \
                \
                iter->link.next = &tmp[new_idx]->link; \
                tmp[new_idx] = iter; \
                \
                iter = next; \
            } \
        } \
        \
        free(self->data); \
        self->data = tmp; \
        self->capacity = new_cap; \
    } \
    static inline f64 concat_layer2(_dict_currLoad_, type)(id self) { \
        return (1.0 * self->size) / self->capacity; \
    } \
    static inline void concat_layer2(_dict_default_emplace_, type)(id self, const i8 *key, type value) { \
        if (!self || !self->data) raise(ERROR, "Cannot put content inside an " RED "empty container." RESET ""); \
        if (!key) { \
            raise(WARNING, "" YELLOW "Invalid key" RESET " (nullptr). Dict elements will remain unchanged."); \
            return; \
        } \
        \
        size_t idx = self->hash(key, strlen(key), self->capacity); \
        \
        Node(Pair(DictKey, type)) iter = self->data[idx]; \
        while (iter) { \
            if (!strcmp(iter->value.first, key)) { \
                Pair(DictKey, type) update = newPair(DictKey, type, {iter->value.first, value}); \
                memcpy((void *)&iter->value, &update, sizeof(iter->value)); \
                return; \
            } \
            iter = getNode(Pair(DictKey, type), iter->link.next); \
        } \
        \
        f64 cl = concat_layer2(_dict_currLoad_, type)(self); \
        if (cl >= self->loadFactor) concat_layer2(dictRehash_, type)(self); \
        \
        Node(Pair(DictKey, type)) entry = newNode(Pair(DictKey, type), newPair(DictKey, type, {strdup(key), value})); \
        \
        entry->link.next = self->data[idx] ? &self->data[idx]->link : nullptr; \
        self->data[idx] = entry; \
        \
        self->size++; \
    } \
    static inline type concat_layer2(_dict_default_get_, type)(id self, const i8 *key) { \
        if (!self || !self->data) raise(ERROR, "Cannot get items from an " RED "empty container." RESET ""); \
        if (!key) { \
            raise(WARNING, "" YELLOW "Invalid key" RESET " (nullptr). Returning nil."); \
            return (type){0}; \
        } \
        \
        size_t idx = self->hash(key, strlen(key), self->capacity); \
        \
        Node(Pair(DictKey, type)) iter = self->data[idx]; \
        while (iter) { \
            if (!strcmp(iter->value.first, key)) { \
                return iter->value.second; \
            } \
            iter = getNode(Pair(DictKey, type), iter->link.next); \
        } \
        \
        raise(WARNING, "Cannot find any " YELLOW "item with key: \"%s\"" RESET ". Returning nil.", key); \
        return (type){0}; \
    } \
    static inline void concat_layer2(_dict_default_put_, type)(id self, Pair(DictKey, type) item) { \
        if (!self || !self->data) raise(ERROR, "Cannot put content inside an " RED "empty container." RESET ""); \
        \
        const i8* key = item.first; \
        if (!key) { \
            raise(WARNING, "" YELLOW "Invalid key" RESET " (nullptr). Dict elements will remain unchanged."); \
            return; \
        } \
        \
        size_t idx = self->hash(key, strlen(key), self->capacity); \
        \
        Node(Pair(DictKey, type)) iter = self->data[idx]; \
        while (iter) { \
            if (!strcmp(iter->value.first, key)) { \
                memcpy((void *)&iter->value, &item, sizeof(iter->value)); \
                return; \
            } \
            iter = getNode(Pair(DictKey, type), iter->link.next); \
        } \
        \
        f64 cl = concat_layer2(_dict_currLoad_, type)(self); \
        if (cl >= self->loadFactor) concat_layer2(dictRehash_, type)(self); \
        \
        Node(Pair(DictKey, type)) entry = newNode(Pair(DictKey, type), newPair(DictKey, type, {strdup(item.first), item.second})); \
        \
        entry->link.next = self->data[idx] ? &self->data[idx]->link : nullptr; \
        self->data[idx] = entry; \
        \
        self->size++; \
    } \
    static inline bool concat_layer2(_dict_default_contains_, type)(id self, const i8 *key) { \
        if (!self || !self->data) raise(ERROR, "Cannot check content inside an " RED "empty container." RESET ""); \
        if (!key) { \
            raise(WARNING, "" YELLOW "Invalid key" RESET " (nullptr). Dict elements will remain unchanged."); \
            return false; \
        } \
        \
        size_t idx = self->hash(key, strlen(key), self->capacity); \
        \
        Node(Pair(DictKey, type)) iter = self->data[idx]; \
        while (iter) { \
            if (!strcmp(iter->value.first, key)) return true; \
            iter = getNode(Pair(DictKey, type), iter->link.next); \
        } \
        \
        return false; \
    } \
    static inline type concat_layer2(_dict_default_remove_, type)(id self, const i8 *key) { \
        if (!self || !self->data) raise(ERROR, "Cannot remove content inside an " RED "empty container." RESET ""); \
        if (!key) { \
            raise(WARNING, "" YELLOW "Invalid key" RESET " (nullptr). Dict elements will remain unchanged."); \
            raise(WARNING, "Returning nil."); \
            return (type){0}; \
        } \
        \
        size_t idx = self->hash(key, strlen(key), self->capacity); \
        bool removed = false; \
        type ret; \
        \
        Node(Pair(DictKey, type)) iter = self->data[idx]; \
        Node(Pair(DictKey, type)) prev = nullptr; \
        while (iter) { \
            if (!strcmp(iter->value.first, key)) { \
                ret = iter->value.second; \
                \
                if (prev) prev->link.next = iter->link.next; \
                else self->data[idx] = getNode(Pair(DictKey, type), iter->link.next); \
                \
                free(iter); \
                self->size--; \
                removed = true; \
                break; \
            } \
            prev = iter; \
            iter = getNode(Pair(DictKey, type), iter->link.next); \
        } \
        \
        if (!removed) { \
            raise(WARNING, "Invalid key: " YELLOW "%s" RESET ". Dict elements will remain unchanged.", key); \
            raise(WARNING, "Returning nil."); \
            return (type){0}; \
        } \
        return ret; \
    } \
    \
    static concat_layer2(DictVTable_, type) concat_layer2(DictVTable_, concat_layer2(dict_, type)) = { \
        .hash = dict_default_hash, \
        .fmt = concat_layer2(_dict_default_fmt_, type), \
        .toString = concat_layer2(_dict_default_toString_, type), \
        .emplace = concat_layer2(_dict_default_emplace_, type), \
        .get = concat_layer2(_dict_default_get_, type), \
        .put = concat_layer2(_dict_default_put_, type), \
        .currLoad = concat_layer2(_dict_currLoad_, type), \
        .contains = concat_layer2(_dict_default_contains_, type), \
        .remove = concat_layer2(_dict_default_remove_, type), \
    }; \

#define DictVTable(type) concat_layer2(DictVTable_, type)
#define DictVTableInstance(type) concat_layer2(DictVTable_, concat_layer2(dict_, type))

#endif

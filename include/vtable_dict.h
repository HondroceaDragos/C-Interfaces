#ifndef VTABLE_DICT_H
#define VTABLE_DICT_H

#include "./helpers.h"
#include "./raise.h"
#include "./node.h"
#include "./list.h"
#include "./tuple.h"
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

const ui64 dict_default_hash(DictKey data, size_t dataSize, size_t dictCap) {
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
    void (*put)(id, Tuple(DictKey, type)); \
    void (*emplace)(id, const i8 *, type); \
    type (*get)(id, const i8 *); \
    Tuple(DictKey, type) (*remove)(id, const i8 *);

#define DictVTableType(id, type) \
    typedef struct concat_layer2(_dict_vtable_, type) { \
        DictVTableFunctions(id, type) \
    } concat_layer2(DictVTable_, type); \
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
        int32_t slen = snprintf(buffer, bcap, "Dict(%zu / %zu = %.2f <= %.2f) {\n", \
            self->size, self->capacity, currLoad, self->loadFactor); \
        bsize += slen; \
        size_t size = self->capacity; \
        \
        for (size_t idx = 0; idx < size; idx++) { \
            LinkedList currList = self->data[idx]; \
            if (!currList->size) continue; \
            \
            bsize += snprintf(buffer + bsize, bcap - bsize, "[%zu]:\n\t", idx); \
            \
            NodeLink *iter = currList->head; \
            for (size_t lidx = 0; lidx < currList->size; lidx++) { \
                Node(Tuple(DictKey, type)) content = getNode(Tuple(DictKey, type), iter); \
                \
                i8 *element = (content->toString != concat_layer2(_node_default_tostring_, Tuple(DictKey, type))) ? \
                    content->toString(content) : self->fmt(content->value->first, content->value->second); \
                if (!element) raise(ERROR, "Out-of-Memory"); \
                size_t elen = strlen(element); \
                \
                const i8 *sep = (lidx + 1 < currList->size) ? " <->\n\t" : "\n"; \
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
                iter = iter->next; \
            } \
        } \
        snprintf(buffer + bsize, bcap - bsize, "}"); \
        \
        return buffer; \
    } \
    static inline void delete(id)(id *d) { \
        if (!d || !*d) return; \
        LinkedList slot = nullptr; \
        for (size_t idx = 0; idx < (*d)->capacity; idx++) { \
            slot = (*d)->data[idx]; \
            NodeLink *iter = slot->head; \
            for (size_t _ = 0; _ < slot->size; _++) { \
                NodeLink *tmp = iter->next; \
                Node(Tuple(DictKey, type)) n = getNode(Tuple(DictKey, type), iter); \
                delete(Node(Tuple(DictKey, type)))(&n); \
                iter = tmp; \
            } \
            delete(LinkedList)(&slot); \
        } \
        free((*d)->data); \
        free(*d); \
        *d = nullptr; \
    } \
    static inline void concat_layer2(_dict_default_emplace_, type)(id self, const i8 *key, type value) { \
        if (!self || !self->data) raise(ERROR, "Cannot put content inside an " RED "empty container." RESET ""); \
        \
        size_t idx = self->hash(key, strlen(key), self->capacity); \
        LinkedList slot = self->data[idx]; \
        NodeLink *iter = slot->head; \
        for (size_t _ = 0; _ < slot->size; _++) { \
            Node(Tuple(DictKey, type)) src = getNode(Tuple(DictKey, type), iter); \
            if (!strcmp(src->value->first, key)) { \
                src->value->second = value; \
                return; \
            } \
            iter = iter->next; \
        } \
        \
        Node(Tuple(DictKey, type)) entry = newNode(Tuple(DictKey, type), newTuple(DictKey, type, strdup(key), value)); \
        \
        slot->push.rear(slot, &entry->link); \
        \
        self->size++; \
    } \
    static concat_layer2(DictVTable_, type) concat_layer2(DictVTable_, concat_layer2(dict_, type)) = { \
        .hash = dict_default_hash, \
        .fmt = concat_layer2(_dict_default_fmt_, type), \
        .toString = concat_layer2(_dict_default_toString_, type), \
        .emplace = concat_layer2(_dict_default_emplace_, type), \
    }; \

#define DictVTable(type) concat_layer2(DictVTable_, type)
#define DictVTableInstance(type) concat_layer2(DictVTable_, concat_layer2(dict_, type))

#endif

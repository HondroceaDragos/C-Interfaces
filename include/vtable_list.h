#ifndef VTABLE_LIST_H
#define VTABLE_LIST_H

#include "./helpers.h"
#include "./raise.h"
#include "./node.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

static inline void *_getNode_primitive(NodeLink *nl, size_t offset) {
    return (nl) ? ((int8_t *)nl) - offset : nullptr;
}

#define getNode(type, _node_link) \
    (Node(type))(_getNode_primitive(_node_link, offsetof(struct concat_layer2(_node_, type), link)))

#define ListVTableFunctions(id) \
    NodeLink *(*at)(id, int32_t); \
    struct { \
        void (*front)(id, NodeLink *); \
        void (*rear)(id, NodeLink *); \
    } push; \
    struct { \
        NodeLink *(*front)(id); \
        NodeLink *(*rear)(id); \
    } pop; \
    void (*reverse)(id); \
    int8_t *(*toString)(id); \
    void (*insert)(id, NodeLink *, int32_t);

#define ListVTableType(id) \
    typedef struct _list_vtable_ { \
        ListVTableFunctions(id) \
    } LinkedListVTable; \
    static inline void delete(id)(id *l) { \
        if (!l || !*l) return; \
        NodeLink *iter = (*l)->head; \
        free(*l); \
        *l = nullptr; \
    } \
    \
    static inline NodeLink *_list_default_at(LinkedList self, int32_t idx) { \
        int32_t size = (int32_t)self->size; \
        if (!size) { \
            raise(WARNING, "List is " YELLOW "empty " RESET "(size = %zu).", self->size); \
            raise(WARNING, "Returning " YELLOW "invalid reference " RESET "(nullptr)."); \
            return nullptr; \
        } \
        if (idx >= size) { \
            raise(WARNING, "Index " YELLOW "out-of-bounds" RESET ". idx = %d is not in range [0, %d].", idx, size); \
            raise(WARNING, "Returning " YELLOW "invalid reference " RESET "(nullptr)."); \
            return nullptr; \
        } \
        if (idx < -size) { \
            raise(WARNING, "Index " YELLOW "out-of-bounds" RESET ". idx = %d is not in range [%d, -1].", idx, -size); \
            raise(WARNING, "Returning " YELLOW "invalid reference " RESET "(nullptr)."); \
            return nullptr; \
        } \
        idx = (idx < 0) ? (size + idx) : idx; \
        NodeLink *iter = self->head; \
        int32_t i = 0; \
        while (i < idx && iter) { \
            iter = iter->next; \
            i++; \
        } \
        \
        return iter; \
    } \
    static inline void _list_default_push_rear(LinkedList self, NodeLink *nl) { \
        if (!self) { \
            raise(ERROR, "Cannot push into an " RED "empty container " RESET "(nil)."); \
        } \
        if (!nl) { \
            raise(WARNING, "Cannot " YELLOW "push value " RESET "(of-type: nil). List elements will remain unchanged."); \
            return; \
        } \
        if (!self->tail) { \
            self->tail = nl; \
            self->head = self->tail; \
            self->size = 1; \
            return; \
        } \
        self->tail->next = nl; \
        nl->prev = self->tail; \
        self->tail = self->tail->next; \
        self->size++; \
    } \
    static inline void _list_default_push_front(LinkedList self, NodeLink *nl) { \
        if (!self) { \
            raise(ERROR, "Cannot push into an " RED "empty container " RESET "(nil)."); \
        } \
        if (!nl) { \
            raise(WARNING, "Cannot " YELLOW "push value " RESET "(of-type: nil). List elements will remain unchanged."); \
            return; \
        } \
        if (!self->head) { \
            self->head = nl; \
            self->tail = self->head; \
            self->size = 1; \
            return; \
        } \
        nl->next = self->head; \
        self->head->prev = nl; \
        self->head = nl; \
        self->size++; \
    } \
    static inline NodeLink *_list_default_pop_rear(LinkedList self) { \
        if (!self) { \
            raise(ERROR, "Cannot pop an " RED "empty container " RESET "(nil)."); \
        } \
        if (!self->tail) { \
            raise(WARNING, "List is " YELLOW "empty " RESET "(size = %zu).", self->size); \
            raise(WARNING, "Returning " YELLOW "invalid reference " RESET "(nullptr)."); \
            return nullptr; \
        } \
        NodeLink *ret = self->tail; \
        self->tail = self->tail->prev; \
        \
        if (self->tail) self->tail->next = nullptr; \
        else self->head = nullptr; \
        \
        self->size--; \
        \
        return ret; \
    } \
    static inline NodeLink *_list_default_pop_front(LinkedList self) { \
        if (!self) { \
            raise(ERROR, "Cannot pop an " RED "empty container " RESET "(nil)."); \
        } \
        if (!self->head) { \
            raise(WARNING, "List is " YELLOW "empty " RESET "(size = %zu).", self->size); \
            raise(WARNING, "Returning " YELLOW "invalid reference " RESET "(nullptr)."); \
            return nullptr; \
        } \
        NodeLink *ret = self->head; \
        self->head = self->head->next; \
        \
        if (self->head) self->head->prev = nullptr; \
        else self->tail = nullptr; \
        \
        self->size--; \
        \
        return ret; \
    } \
    static inline void _list_default_reverse(LinkedList self) { \
        if (!self) raise(ERROR, "Cannot reverse an " RED "empty container " RESET "(nil)."); \
        if (!self->head) raise(WARNING, "List is " YELLOW "empty " RESET "(size = %zu).", self->size); \
        \
        NodeLink *iter = self->tail; \
        size_t idx = 0; \
        while (idx < self->size && iter) { \
            NodeLink *tmp = iter->prev; \
            iter->prev = iter->next; \
            iter->next = tmp; \
            idx++; \
            iter = tmp; \
        } \
        \
        iter = self->head; \
        self->head = self->tail; \
        self->tail = iter; \
    } \
    static inline int8_t *_list_default_tostring(id self) { \
        size_t bsize = 0; \
        size_t bcap = 2048; \
        \
        int8_t *buffer = calloc(bcap, sizeof(*buffer)); \
        buffer[0] = '\0'; \
        \
        int32_t slen = snprintf(buffer, bcap, "List(%zu) {\n\t(nil) <-\n\t", self->size); \
        bsize += slen; \
        size_t size = self->size; \
        \
        NodeLink *iter = self->head; \
        for (size_t idx = 0; idx < size; idx++) { \
            int8_t *element = calloc(256, sizeof(*element)); \
            if (!element) { \
                raise(ERROR, "Die"); \
            } \
            snprintf(element, 256, "(Addr: %p)", iter); \
            size_t elen = strlen(element); \
            \
            const int8_t *sep = (idx + 1 < size) ? " <->\n\t" : " ->\n\t"; \
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
                    raise(ERROR, "Cannot build list string " RED "(out-of-memory)" RESET ". List elements will remain unchanged."); \
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
        \
        snprintf(buffer + bsize, bcap - bsize, "(nil)\n}"); \
        return buffer; \
    } \
    static inline void _list_default_insert(id self, NodeLink *value, int32_t idx) { \
        int32_t size = (int32_t)self->size; \
        if (!size || !self->head) { \
            self->head = value; \
            self->tail = self->head; \
            self->size = 1; \
            return; \
        } \
        if (idx > size) { \
            raise(WARNING, "Index " YELLOW "out-of-bounds" RESET ". idx = %d is not in range [0, %d].", idx, size); \
            raise(WARNING, "List elements will remain unchanged."); \
            return; \
        } \
        if (idx < -size) { \
            raise(WARNING, "Index " YELLOW "out-of-bounds" RESET ". idx = %d is not in range [0, %d].", idx, size); \
            raise(WARNING, "List elements will remain unchanged."); \
            return; \
        } \
        \
        NodeLink *prev = nullptr; \
        NodeLink *next = nullptr; \
        \
        if (idx == size) prev = self->tail; \
        else if (idx >= 0) { \
            next = self->head; \
            for (int32_t i = 0; i < idx; i++) next = next->next; \
            prev = next->prev; \
        } else { \
            next = self->tail; \
            for (int32_t i = -1; i > idx; i--) next = next->prev; \
            prev = next->prev; \
        } \
        \
        value->prev = prev; \
        value->next = next; \
        \
        if (prev) prev->next = value; \
        else self->head = value; \
        \
        if (next) next->prev = value; \
        else self->tail = value; \
        \
        self->size++; \
    } \
    static LinkedListVTable LinkedListVTableInstance = { \
        .at = _list_default_at, \
        .push.rear = _list_default_push_rear, \
        .push.front = _list_default_push_front, \
        .pop.rear = _list_default_pop_rear, \
        .pop.front = _list_default_pop_front, \
        .reverse = _list_default_reverse, \
        .toString = _list_default_tostring, \
        .insert = _list_default_insert, \
    }; \

#endif

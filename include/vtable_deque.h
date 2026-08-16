#ifndef VTABLE_DEQUE_H
#define VTABLE_DEQUE_H

#include "./helpers.h"
#include "./raise.h"
#include "./node.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#define DequeVTableFunctions(id, type) \
    struct { \
        void (*front)(id, type); \
        void (*rear)(id, type); \
    } push; \
    struct { \
        type (*front)(id); \
        type (*rear)(id); \
    } pop; \
    int8_t *(*fmt)(const type); \
    int8_t *(*toString)(id); \
    bool (*empty)(id); \
    void (*clear)(id, MemoryCleanup); \
    bool (*reachable)(id); \
    struct { \
        type (*front)(id); \
        type (*rear)(id); \
    } peek; \

#define DequeVTableType(id, type) \
    typedef struct concat_layer2(_deque_vtable_, type) { \
        DequeVTableFunctions(id, type) \
    } concat_layer2(DequeVTable_, type); \
    static inline int8_t *concat_layer2(_deque_default_fmt_, type)(const type) { \
        raise(WARNING, "Deque uses " YELLOW "default formatting function " RESET "(cannot deduce type)."); \
        return strdup("[?]"); \
    } \
    static inline void delete(id)(id *dq) { \
        if (!dq || !*dq) return; \
        NodeLink *iter = &(*dq)->head->link; \
        for (size_t idx = 0; idx < (*dq)->size; idx++) { \
            Node(type) tmp = getNode(type, iter); \
            iter = iter->next; \
            delete(Node(type))(&tmp); \
        } \
        free(*dq); \
        *dq = nullptr; \
    } \
    static inline void concat_layer2(_deque_default_clear_, type)(id self, MemoryCleanup mc) { \
        if (mc == FREE_MEMORY) { \
            NodeLink *iter = &self->head->link; \
            for (size_t idx = 0; idx < self->size; idx++) { \
                Node(type) tmp = getNode(type, iter); \
                iter = iter->next; \
                delete(Node(type))(&tmp); \
            } \
        } \
        self->head = self->tail = nullptr; \
        self->size = 0; \
    } \
    static inline void concat_layer2(_deque_default_push_rear, type)(id self, type elem) { \
        if (!self) raise(ERROR, "Cannot push into an " RED "empty container " RESET "(nil)."); \
        \
        Node(type) n = newNode(type, elem); \
        n->fmt = self->fmt; \
        \
        if (!self->tail) { \
            self->tail = n; \
            self->head = n; \
            self->size = 1; \
            return; \
        } \
        \
        n->link.prev = &self->tail->link; \
        self->tail->link.next = &n->link; \
        self->tail = n; \
        self->size++; \
    } \
    static inline void concat_layer2(_deque_default_push_front, type)(id self, type elem) { \
        if (!self) raise(ERROR, "Cannot push into an " RED "empty container " RESET "(nil)."); \
        \
        Node(type) n = newNode(type, elem); \
        n->fmt = self->fmt; \
        \
        if (!self->head) { \
            self->head = n; \
            self->tail = n; \
            self->size = 1; \
            return; \
        } \
        \
        n->link.next = &self->head->link; \
        self->head->link.prev = &n->link; \
        self->head = n; \
        self->size++; \
    } \
    static inline type concat_layer2(_deque_default_pop_rear, type)(id self) { \
        if (!self) raise(ERROR, "Cannot pop an " RED "empty container " RESET "(nil)."); \
        if (!self->tail) { \
            raise(WARNING, "Deque is " YELLOW "empty " RESET "(size = %zu).", self->size); \
            raise(WARNING, "Returning " YELLOW "invalid reference " RESET "(zero)."); \
            return (type){0}; \
        } \
        \
        Node(type) n = self->tail; \
        type ret = n->value; \
        \
        if (self->size == 1) { \
            self->head = self->tail = nullptr; \
        } else { \
            self->tail = getNode(type, self->tail->link.prev); \
            self->tail->link.next = nullptr; \
        } \
        \
        delete(Node(type))(&n); \
        self->size--; \
        \
        return ret; \
    } \
    static inline type concat_layer2(_deque_default_pop_front, type)(id self) { \
        if (!self) raise(ERROR, "Cannot pop an " RED "empty container " RESET "(nil)."); \
        if (!self->tail) { \
            raise(WARNING, "Deque is " YELLOW "empty " RESET "(size = %zu).", self->size); \
            raise(WARNING, "Returning " YELLOW "invalid reference " RESET "(zero)."); \
            return (type){0}; \
        } \
        \
        Node(type) n = self->head; \
        type ret = n->value; \
        \
        if (self->size == 1) { \
            self->head = self->tail = nullptr; \
        } else { \
            self->head = getNode(type, self->head->link.next); \
            self->head->link.prev = nullptr; \
        } \
        \
        delete(Node(type))(&n); \
        self->size--; \
        \
        return ret; \
    } \
    static inline int8_t *concat_layer2(_deque_default_tostring_, type)(id self) { \
        if (!self) raise(ERROR, "Cannot format an " RED "empty container " RESET "(nil)."); \
        size_t bsize = 0; \
        size_t bcap = 256; \
        \
        int8_t *buffer = calloc(bcap, sizeof(*buffer)); \
        buffer[0] = '\0'; \
        \
        int32_t slen = snprintf(buffer, bcap, "Deque(%zu) {\n\t(nil) <-\n[head]:", self->size); \
        bsize += slen; \
        size_t size = self->size; \
        \
        NodeLink *iter = &self->head->link; \
        for (size_t idx = 0; idx < size; idx++) { \
            Node(type) n = getNode(type, iter); \
            int8_t *element = n->fmt(n->value); \
            size_t elen = strlen(element); \
            \
            const int8_t *start = (idx != size - 1) ? "\t" : "[tail]: "; \
            size_t stlen = strlen(start); \
            \
            const int8_t *sep = (idx + 1 < size) ? " <->\n" : " ->\n\t"; \
            size_t seplen = strlen(sep); \
            \
            size_t space = bsize + elen + seplen + 1 + 8 + stlen; \
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
            bsize += snprintf(buffer + bsize, bcap - bsize, "%s%s%s", start, element, sep); \
            free(element); \
            iter = iter->next; \
        } \
        \
        snprintf(buffer + bsize, bcap - bsize, "(nil)\n}"); \
        return buffer; \
    } \
    static inline bool concat_layer2(_deque_default_empty_, type)(id self) { \
        return (self->size == 0); \
    } \
    static inline bool concat_layer2(_deque_default_reachable_, type)(id self) { \
        return (self->head || self->tail); \
    } \
    static inline type concat_layer2(_deque_default_peek_rear, type)(id self) { \
        if (self->size && self->tail) return self->tail->value; \
        raise(WARNING, "Cannot peek " YELLOW "(deque empty)" RESET ". Returning zero."); \
        return (type){0}; \
    } \
    static inline type concat_layer2(_deque_default_peek_front, type)(id self) { \
        if (self->size && self->head) return self->head->value; \
        raise(WARNING, "Cannot peek " YELLOW "(deque empty)" RESET ". Returning zero."); \
        return (type){0}; \
    } \
    static concat_layer2(DequeVTable_, type) concat_layer2(DequeVTable_, concat_layer2(deque_, type)) = { \
        .push.front = concat_layer2(_deque_default_push_front, type), \
        .push.rear = concat_layer2(_deque_default_push_rear, type), \
        .pop.front = concat_layer2(_deque_default_pop_front, type), \
        .pop.rear = concat_layer2(_deque_default_pop_rear, type), \
        .fmt = concat_layer2(_deque_default_fmt_, type), \
        .toString = concat_layer2(_deque_default_tostring_, type), \
        .empty = concat_layer2(_deque_default_empty_, type), \
        .clear = concat_layer2(_deque_default_clear_, type), \
        .reachable = concat_layer2(_deque_default_reachable_, type), \
        .peek.front = concat_layer2(_deque_default_peek_front, type), \
        .peek.rear = concat_layer2(_deque_default_peek_rear, type), \
    }; \

#define DequeVTable(type) concat_layer2(DequeVTable_, type)
#define DequeVTableInstance(type) concat_layer2(DequeVTable_, concat_layer2(deque_, type))

#endif

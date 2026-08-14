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
        self->size = 0; \
    } \
    static inline void concat_layer2(_deque_default_push_, type)(id self, type elem) { \
        return; \
    } \
    static inline type concat_layer2(_deque_default_pop_, type)(id self) { \
        return (type){0}; \
    } \
    static inline int8_t *concat_layer2(_deque_default_tostring_, type)(id self) { \
        return strdup("Not implemented."); \
    } \
    static inline bool concat_layer2(_deque_default_empty_, type)(id self) { \
        return (self->size == 0); \
    } \
    static inline bool concat_layer2(_deque_default_reachable_, type)(id self) { \
        return (self->head || self->tail); \
    } \
    static concat_layer2(DequeVTable_, type) concat_layer2(DequeVTable_, concat_layer2(deque_, type)) = { \
        .push.front = concat_layer2(_deque_default_push_, type), \
        .push.rear = concat_layer2(_deque_default_push_, type), \
        .pop.front = concat_layer2(_deque_default_pop_, type), \
        .pop.rear = concat_layer2(_deque_default_pop_, type), \
        .fmt = concat_layer2(_deque_default_fmt_, type), \
        .toString = concat_layer2(_deque_default_tostring_, type), \
        .empty = concat_layer2(_deque_default_empty_, type), \
        .clear = concat_layer2(_deque_default_clear_, type), \
        .reachable = concat_layer2(_deque_default_reachable_, type), \
    }; \

#define DequeVTable(type) concat_layer2(DequeVTable_, type)
#define DequeVTableInstance(type) concat_layer2(DequeVTable_, concat_layer2(deque_, type))

#endif

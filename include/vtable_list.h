#ifndef VTABLE_LIST_H
#define VTABLE_LIST_H

#include "./helpers.h"
#include "./raise.h"
#include "./node.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#define getNode(type, _node_link) \
    (_node_link) ? (Node(type))((int8_t *)(_node_link) - offsetof(struct concat_layer2(_node_, type), link)) : nullptr

#define ListVTableFunctions(id) \
    NodeLink *(*at)(id, int32_t); \
    struct { \
        void (*front)(id, NodeLink *); \
        void (*rear)(id, NodeLink *); \
    } push;

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
    static inline NodeLink *_list_default_at(LinkedList l, int32_t idx) { \
        NodeLink *iter = l->head; \
        int32_t i = 0; \
        while (iter && i < idx) { \
            iter = iter->next; \
            i++; \
        } \
        \
        return iter; \
    } \
    static inline void _list_default_push_rear(LinkedList l, NodeLink *nl) { \
        if (!l) { \
            raise(ERROR, "Cannot push into an " RED "empty container " RESET "(nil)."); \
        } \
        if (!nl) { \
            raise(WARNING, "Cannot " YELLOW "push value " RESET "(of-type: nil). List elements will remain unchanged."); \
            return; \
        } \
        if (!l->tail) { \
            l->tail = nl; \
            l->head = l->tail; \
            l->size = 1; \
            return; \
        } \
        l->tail->next = nl; \
        nl->prev = l->tail; \
        l->tail = l->tail->next; \
        l->size++; \
    } \
    static inline void _list_default_push_front(LinkedList l, NodeLink *nl) { \
        if (!l) { \
            raise(ERROR, "Cannot push into an " RED "empty container " RESET "(nil)."); \
        } \
        if (!nl) { \
            raise(WARNING, "Cannot " YELLOW "push value " RESET "(of-type: nil). List elements will remain unchanged."); \
            return; \
        } \
        if (!l->head) { \
            l->head = nl; \
            l->tail = l->head; \
            l->size = 1; \
            return; \
        } \
        nl->next = l->head; \
        l->head->prev = nl; \
        l->head = nl; \
        l->size++; \
    } \
    static LinkedListVTable LinkedListVTableInstance = { \
        .at = _list_default_at, \
        .push.rear = _list_default_push_rear, \
        .push.front = _list_default_push_front, \
    }; \

#endif

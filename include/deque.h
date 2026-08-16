#ifndef DEQUE_H
#define DEQUE_H

#include "./array.h"
#include "./vtable_deque.h"
#include "./raise.h"
#include "./iterator.h"
#include "./node.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void *_deque_next(Iterator i, size_t) {
    NodeLink *ni = (NodeLink *)i->ref;
    return ni->next;
}

#define DequeType(type) \
    typedef struct concat_layer2(_deque_, type) { \
        Node(type) head; \
        Node(type) tail; \
        size_t size; \
        \
        DequeVTableFunctions(struct concat_layer2(_deque_, type) *, type) \
    } *concat_layer2(Deque_, type); \
    \
    DequeVTableType(concat_layer2(Deque_, type), type) \
    \
    struct concat_layer2(_deque_defaults_, type) { \
        Array(type) using; \
        DequeVTableFunctions(concat_layer2(Deque_, type), type) \
    }; \
    \
    static inline concat_layer2(Deque_, type) concat_layer2(newDeque_, type)(struct concat_layer2(_deque_defaults_, type) defaults) { \
        concat_layer2(Deque_, type) dq = calloc(1, sizeof(*dq)); \
        if (!dq) { \
            raise(ERROR, "Cannot create deque " RED "(out-of-memory)" RESET ". Returning nullptr."); \
            return nullptr; \
        } \
        \
        dq->fmt = (defaults.fmt) ? defaults.fmt : DequeVTableInstance(type).fmt; \
        \
        if (defaults.using.data) { \
            type *data = defaults.using.data; \
            size_t size = defaults.using.size; \
            \
            if (size) { \
                Node(type) head = newNode(type, data[0], .fmt = dq->fmt); \
                Node(type) tail = newNode(type, data[size - 1], .fmt = dq->fmt); \
                dq->head = head; \
                dq->tail = tail; \
                \
                NodeLink *iter = &head->link; \
                for (size_t idx = 1; idx < size - 1; idx++) { \
                    Node(type) n = newNode(type, defaults.using.data[idx], .fmt = dq->fmt); \
                    n->link.prev = iter; \
                    iter->next = &n->link; \
                    iter = &n->link; \
                } \
                tail->link.prev = iter; \
                iter->next = &tail->link; \
                dq->size = size; \
            } \
        } \
        \
        dq->toString = (defaults.toString) ? defaults.toString : DequeVTableInstance(type).toString; \
        \
        dq->push.front = (defaults.push.front) ? defaults.push.front : DequeVTableInstance(type).push.front; \
        dq->push.rear = (defaults.push.rear) ? defaults.push.rear : DequeVTableInstance(type).push.rear; \
        \
        dq->pop.front = (defaults.pop.front) ? defaults.pop.front : DequeVTableInstance(type).pop.front; \
        dq->pop.rear = (defaults.pop.rear) ? defaults.pop.rear : DequeVTableInstance(type).pop.rear; \
        \
        dq->clear = (defaults.clear) ? defaults.clear : DequeVTableInstance(type).clear; \
        dq->empty = (defaults.empty) ? defaults.empty : DequeVTableInstance(type).empty; \
        \
        dq->reachable = (defaults.reachable) ? defaults.reachable : DequeVTableInstance(type).reachable; \
        dq->peek.front = (defaults.peek.front) ? defaults.peek.front : DequeVTableInstance(type).peek.front; \
        dq->peek.rear = (defaults.peek.rear) ? defaults.peek.rear : DequeVTableInstance(type).peek.rear; \
        return dq; \
    } \

#define Deque(type) concat_layer2(Deque_, type)
#define newDeque(type, ...) \
    concat_layer2(newDeque_, type)((struct concat_layer2(_deque_defaults_, type)){__VA_ARGS__})

#define forDeque_primitive(i, once, l, acc) \
    for (Iterator i = newIterator(&(l)->head->link, (l)->size); i && i->size; iterator_advance(&i, _deque_next, 0)) \
        for (acc = &((typeof(*(l)->head) *)_getNode_primitive(i->ref, offsetof(typeof(*(l)->head), link)))->value, *once = (void *)1; once; once = 0)

#define forDeque(acc, l) \
    forDeque_primitive(concat_layer2(_i_, __COUNTER__), concat_layer2(_once_, __COUNTER__), l, acc)

#endif

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

static inline void *_deque_next(Iterator i, size_t esize) {
    return nullptr;
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
        return dq; \
    } \

#define Deque(type) concat_layer2(Deque_, type)
#define newDeque(type, ...) \
    concat_layer2(newDeque_, type)((struct concat_layer2(_deque_defaults_, type)){__VA_ARGS__})

#endif

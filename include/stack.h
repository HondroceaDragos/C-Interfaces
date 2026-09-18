#ifndef STACK_H
#define STACK_H

#include "./array.h"
#include "./vtable_stack.h"
#include "./raise.h"
#include "./iterator.h"
#include "./node.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void *_stack_next(Iterator i, size_t) {
    NodeLink *ni = (NodeLink *)i->ref;
    return ni->next;
}

#define StackType(type) \
    typedef struct concat_layer2(_stack_, type) { \
        Node(type) top; \
        size_t size; \
        \
        StackVTableFunctions(struct concat_layer2(_stack_, type) *, type) \
    } *concat_layer2(Stack_, type); \
    \
    StackVTableType(concat_layer2(Stack_, type), type) \
    \
    struct concat_layer2(_stack_defaults_, type) { \
        Array(type) using; \
        StackVTableFunctions(concat_layer2(Stack_, type), type) \
    }; \
    \
    static inline concat_layer2(Stack_, type) concat_layer2(newStack_, type)(struct concat_layer2(_stack_defaults_, type) defaults) { \
        concat_layer2(Stack_, type) st = calloc(1, sizeof(*st)); \
        if (!st) { \
            raise(ERROR, "Cannot create stack " RED "(out-of-memory)" RESET ". Returning nullptr."); \
            return nullptr; \
        } \
        \
        st->fmt = (defaults.fmt) ? defaults.fmt : StackVTableInstance(type).fmt; \
        \
        if (defaults.using.data) { \
            type *data = defaults.using.data; \
            size_t size = defaults.using.size; \
            \
            if (size) { \
                Node(type) head = newNode(type, data[0], .fmt = st->fmt); \
                st->top = head; \
                \
                NodeLink *iter = &head->link; \
                for (size_t idx = 1; idx < size - 1; idx++) { \
                    Node(type) n = newNode(type, defaults.using.data[idx], .fmt = st->fmt); \
                    n->link.prev = iter; \
                    iter->next = &n->link; \
                    iter = &n->link; \
                } \
                st->size = size; \
            } \
        } \
        \
        st->toString = (defaults.toString) ? defaults.toString : StackVTableInstance(type).toString; \
        \
        st->push = (defaults.push) ? defaults.push : StackVTableInstance(type).push; \
        st->pop = (defaults.pop) ? defaults.pop : StackVTableInstance(type).pop; \
        \
        st->clear = (defaults.clear) ? defaults.clear : StackVTableInstance(type).clear; \
        st->empty = (defaults.empty) ? defaults.empty : StackVTableInstance(type).empty; \
        st->peek = (defaults.peek) ? defaults.peek : StackVTableInstance(type).peek; \
        \
        return st; \
    } \

#define Stack(type) concat_layer2(Stack_, type)
#define newStack(type, ...) \
    concat_layer2(newStack_, type)((struct concat_layer2(_stack_defaults_, type)){__VA_ARGS__})

#define forStack_primitive(i, once, l, acc) \
    for (Iterator i = newIterator(&(l)->top->link, (l)->size); i && i->size; iterator_advance(&i, _stack_next, 0)) \
        for (acc = &((typeof(*(l)->top) *)_getNode_primitive(i->ref, offsetof(typeof(*(l)->top), link)))->value, *once = (void *)1; once; once = 0)

#define forStack(acc, l) \
    forStack_primitive(concat_layer2(_i_, __COUNTER__), concat_layer2(_once_, __COUNTER__), l, acc)

#endif

#ifndef VTABLE_STACK_H
#define VTABLE_STACK_H

#include "./helpers.h"
#include "./raise.h"
#include "./node.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#define StackVTableFunctions(id, type) \
    void (*push)(id, type); \
    type (*peek)(id); \
    type (*pop)(id); \
    int8_t *(*fmt)(const type); \
    int8_t *(*toString)(id); \
    bool (*empty)(id); \
    void (*clear)(id, MemoryCleanup); \

#define StackVTableType(id, type) \
    typedef struct concat_layer2(_stack_vtable_, type) { \
        StackVTableFunctions(id, type) \
    } concat_layer2(StackVTable_, type); \
    static inline int8_t *concat_layer2(_stack_default_fmt_, type)(const type) { \
        raise(WARNING, "Stack uses " YELLOW "default formatting function " RESET "(cannot deduce type)."); \
        return strdup("[?]"); \
    } \
    static inline void delete(id)(id *stack) { \
        if (!stack || !*stack) return; \
        NodeLink *iter = &(*stack)->top->link; \
        for (size_t idx = 0; idx < (*stack)->size; idx++) { \
            Node(type) tmp = getNode(type, iter); \
            iter = iter->next; \
            delete(Node(type))(&tmp); \
        } \
        free(*stack); \
        *stack = nullptr; \
    } \
    static inline void concat_layer2(_stack_default_clear_, type)(id self, MemoryCleanup mc) { \
        if (mc == FREE_MEMORY) { \
            NodeLink *iter = &self->top->link; \
            for (size_t idx = 0; idx < self->size; idx++) { \
                Node(type) tmp = getNode(type, iter); \
                iter = iter->next; \
                delete(Node(type))(&tmp); \
            } \
        } \
        self->top = nullptr; \
        self->size = 0; \
    } \
    static inline void concat_layer2(_stack_default_push, type)(id self, type elem) { \
        if (!self) raise(ERROR, "Cannot push into an " RED "empty container " RESET "(nil)."); \
        \
        Node(type) n = newNode(type, elem); \
        n->fmt = self->fmt; \
        \
        if (!self->top) { \
            self->top = n; \
            self->size = 1; \
            return; \
        } \
        \
        n->link.next = &self->top->link; \
        self->top->link.prev = &n->link; \
        self->top = n; \
        self->size++; \
    } \
    static inline type concat_layer2(_stack_default_pop, type)(id self) { \
        if (!self) raise(ERROR, "Cannot pop an " RED "empty container " RESET "(nil)."); \
        if (!self->top) { \
            raise(WARNING, "Stack is " YELLOW "empty " RESET "(size = %zu).", self->size); \
            raise(WARNING, "Returning " YELLOW "invalid reference " RESET "(zero)."); \
            return (type){0}; \
        } \
        \
        Node(type) n = self->top; \
        type ret = n->value; \
        \
        if (self->size == 1) { \
            self->top = nullptr; \
        } else { \
            self->top = getNode(type, self->top->link.next); \
            self->top->link.prev = nullptr; \
        } \
        \
        free(n); \
        self->size--; \
        \
        return ret; \
    } \
    static inline int8_t *concat_layer2(_stack_default_tostring_, type)(id self) { \
        if (!self) raise(ERROR, "Cannot format an " RED "empty container " RESET "(nil)."); \
        size_t bsize = 0; \
        size_t bcap = 256; \
        \
        int8_t *buffer = calloc(bcap, sizeof(*buffer)); \
        buffer[0] = '\0'; \
        \
        int32_t slen = snprintf(buffer, bcap, "Stack(%zu) {\n\t(nil) <-\n[top]:", self->size); \
        bsize += slen; \
        size_t size = self->size; \
        \
        NodeLink *iter = &self->top->link; \
        for (size_t idx = 0; idx < size; idx++) { \
            Node(type) n = getNode(type, iter); \
            int8_t *element = n->fmt(n->value); \
            size_t elen = strlen(element); \
            \
            const int8_t *start = "\t"; \
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
                    raise(ERROR, "Cannot build stack string " RED "(out-of-memory)" RESET ". List elements will remain unchanged."); \
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
    static inline bool concat_layer2(_stack_default_empty_, type)(id self) { \
        return (self->size == 0); \
    } \
    static inline type concat_layer2(_stack_default_peek, type)(id self) { \
        if (self->size && self->top) return self->top->value; \
        raise(WARNING, "Cannot peek " YELLOW "(stack empty)" RESET ". Returning zero."); \
        return (type){0}; \
    } \
    static concat_layer2(StackVTable_, type) concat_layer2(StackVTable_, concat_layer2(stack_, type)) = { \
        .push = concat_layer2(_stack_default_push, type), \
        .pop = concat_layer2(_stack_default_pop, type), \
        .fmt = concat_layer2(_stack_default_fmt_, type), \
        .toString = concat_layer2(_stack_default_tostring_, type), \
        .empty = concat_layer2(_stack_default_empty_, type), \
        .clear = concat_layer2(_stack_default_clear_, type), \
        .peek = concat_layer2(_stack_default_peek, type), \
    }; \

#define StackVTable(type) concat_layer2(StackVTable_, type)
#define StackVTableInstance(type) concat_layer2(StackVTable_, concat_layer2(stack_, type))

#endif

#ifndef VTABLE_NODE_H
#define VTABLE_NODE_H

#include "./helpers.h"
#include "./raise.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#define NodeVTableFunctions(id, type) \
    int8_t *(*fmt)(const type); \
    int8_t *(*toString)(id); \

#define NodeVTableType(id, type) \
    typedef struct concat_layer2(_node_vtable_, type) { \
        NodeVTableFunctions(id, type) \
    } concat_layer2(NodeVTable_, type); \
    static inline int8_t *concat_layer2(_node_default_fmt_, type)(const type) { \
        raise(WARNING, "Node uses " YELLOW "default formatting function " RESET "(cannot deduce type)."); \
        return strdup("[?]"); \
    } \
    static inline void delete(id)(id *n) { \
        if (!n || !*n) return; \
        delete(type)(&(*n)->value); \
        free(*n); \
        *n = nullptr; \
    } \
    static inline int8_t *concat_layer2(_node_default_tostring_, type)(id self) { \
        size_t bsize = 0; \
        size_t bcap = 256; \
        \
        int8_t *buffer = calloc(bcap, sizeof(*buffer)); \
        buffer[0] = '\0'; \
        \
        int8_t *fmt = self->fmt(self->value); \
        size_t header = snprintf(buffer, bcap, "Addr: %p\n", &self->link); \
        snprintf(buffer + header, bcap - header, "Node(value=%s, prev=%p, next=%p)", fmt, self->link.prev, self->link.next); \
        free(fmt); \
        return buffer; \
    } \
    static concat_layer2(NodeVTable_, type) concat_layer2(NodeVTable_, concat_layer2(node_, type)) = { \
        .fmt = concat_layer2(_node_default_fmt_, type), \
        .toString = concat_layer2(_node_default_tostring_, type), \
    }; \

#define NodeVTable(type) concat_layer2(NodeVTable_, type)
#define NodeVTableInstance(type) concat_layer2(NodeVTable_, concat_layer2(node_, type))

#endif

#ifndef NODE_H
#define NODE_H

#include "./raise.h"
#include "./helpers.h"
#include "./vtable_node.h"
#include <stdlib.h>
#include <stddef.h>

typedef struct _node_link NodeLink;
struct _node_link {
    NodeLink *next;
    NodeLink *prev;
};

static inline NodeLink newNodeLink(struct _node_link defaults) {
    return (NodeLink) {
        .next = defaults.next,
        .prev = defaults.prev,
    };
}

static inline void *_getNode_primitive(NodeLink *nl, size_t offset) {
    return (nl) ? ((int8_t *)nl) - offset : nullptr;
}

#define getNode(type, _node_link) \
    (Node(type))(_getNode_primitive(_node_link, offsetof(struct concat_layer2(_node_, type), link)))

#define NodeType(type) \
    typedef struct concat_layer2(_node_, type) { \
        type value; \
        NodeLink link; \
        \
        NodeVTableFunctions(struct concat_layer2(_node_, type) *, type) \
    } *concat_layer2(Node_, type); \
    \
    NodeVTableType(concat_layer2(Node_, type), type) \
    \
    static inline concat_layer2(Node_, type) concat_layer2(newNode_, type)(struct concat_layer2(_node_, type) defaults) { \
        concat_layer2(Node_, type) n = calloc(1, sizeof(*n)); \
        if (!n) { \
            raise(ERROR, "Cannot " RED "create node " RESET "(out-of-memory)."); \
        } \
        \
        n->value = defaults.value; \
        n->link =  (defaults.link.next || defaults.link.prev) ? defaults.link : newNodeLink((struct _node_link){0}); \
        \
        n->fmt = (defaults.fmt) ? defaults.fmt : NodeVTableInstance(type).fmt; \
        n->toString = (defaults.toString) ? defaults.toString : NodeVTableInstance(type).toString; \
        \
        return n;  \
    }

#define Node(type) concat_layer2(Node_, type)
#define newNode(type, ...) concat_layer2(newNode_, type)((struct concat_layer2(_node_, type)){__VA_ARGS__})

#define link(...) newNodeLink((struct _node_link){__VA_ARGS__})

#endif

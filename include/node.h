#ifndef NODE_H
#define NODE_H

#include "./raise.h"
#include "./helpers.h"
#include <stdlib.h>

typedef struct _node_link *NodeLink;
struct _node_link {
    NodeLink next;
    NodeLink prev;
};

static inline NodeLink newNodeLink(struct _node_link defaults) {
    NodeLink nl = (NodeLink)calloc(1, sizeof(*nl));
    if (!nl) {
        raise(ERROR, "Cannot " RED "create node " RESET "(out-of-memory).");
    }

    nl->next = (defaults.next) ? defaults.next : nullptr;
    nl->prev = (defaults.prev) ? defaults.prev : nullptr;

    return nl;
}

deleteDefine(NodeLink) {
    if (!self || !*self) return;
    free(*self);
    *self = nullptr;
}

#define NodeType(type) \
    typedef struct concat_layer2(_node_, type) { \
        type value; \
        NodeLink link; \
    } *concat_layer2(Node_, type); \
    \
    static inline void delete(concat_layer2(Node_, type))(concat_layer2(Node_, type) *n) { \
        if (!n || !*n) return; \
        delete(type)(&(*n)->value); \
        delete(NodeLink)(&(*n)->link); \
        free(*n); \
        *n = nullptr; \
    } \
    static inline concat_layer2(Node_, type) concat_layer2(newNode_, type)(struct concat_layer2(_node_, type) defaults) { \
        concat_layer2(Node_, type) n = calloc(1, sizeof(*n)); \
        if (!n) { \
            raise(ERROR, "Cannot " RED "create node " RESET "(out-of-memory)."); \
        } \
        \
        n->value = (defaults.value) ? defaults.value : (type){0}; \
        n->link = (defaults.link) ? defaults.link : newNodeLink((struct _node_link){0}); \
        \
        return n;  \
    }

#define Node(type) concat_layer2(Node_, type)
#define newNode(type, ...) concat_layer2(newNode_, type)((struct concat_layer2(_node_, type)){__VA_ARGS__})

#define link(...) newNodeLink((struct _node_link){__VA_ARGS__})

#endif

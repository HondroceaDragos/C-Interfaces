#ifndef LIST_H
#define LIST_H

#include "./node.h"
#include "./iterator.h"
#include "./vtable_list.h"
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

static inline void *_linked_list_next(Iterator i, size_t) {
    NodeLink *ni = (NodeLink *)i->ref;
    return ni->next;
}

typedef struct _linked_list {
    NodeLink *head;
    NodeLink *tail;
    size_t size;

    ListVTableFunctions(struct _linked_list *);
} *LinkedList;

ListVTableType(LinkedList)

LinkedList _newLinkedList(struct _linked_list defaults) {
    LinkedList l = (LinkedList)calloc(1, sizeof(*l));
    if (!l) {
        raise(ERROR, "Cannot create list " RED "(out-of-memory)" RESET ".");
    }

    l->head = defaults.head;
    l->tail = defaults.tail;
    l->size = (l->head) ? 1 : 0;

    l->at = (defaults.at) ? defaults.at : LinkedListVTableInstance.at;
    l->push.rear = (defaults.push.rear) ? defaults.push.rear : LinkedListVTableInstance.push.rear;
    l->push.front = (defaults.push.front) ? defaults.push.front : LinkedListVTableInstance.push.front;

    return l;
}

#define newLinkedList(...) _newLinkedList((struct _linked_list){__VA_ARGS__})

#define forLinkedList_primitive(i, once, l, acc) \
    for (Iterator i = newIterator((l)->head, (l)->size); i && i->size; iterator_advance(&i, _linked_list_next, 0)) \
        for (acc = (typeof(*(l)->head) *)i->ref, *once = (void *)1; once; once = 0)

#define forLinkedList(acc, l) \
    raise(WARNING, "Using " YELLOW "iterator " RESET "over a " YELLOW "list " RESET "causes " YELLOW "Undefined Behaviour" RESET "."); \
    forLinkedList_primitive(concat_layer2(_i_, __COUNTER__), concat_layer2(_once_, __COUNTER__), l, acc)

#endif

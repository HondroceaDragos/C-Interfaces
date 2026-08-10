#ifndef ITERATOR_H
#define ITERATOR_H

#include "./helpers.h"
#include "./raise.h"

typedef struct _iterator {
    void *ref;
    size_t step;
    size_t size;
} *Iterator;

typedef void *(*IteratorNext)(Iterator, size_t esize);

static inline Iterator newIterator(void *data, size_t size) {
    Iterator i = (Iterator)calloc(1, sizeof(*i));
    if (!i) {
        raise(ERROR, "Cannot define " RED "iterator " RESET "(out-of-memory).");
    }
    i->ref = data;
    i->size = size;
    return i;
}

static inline bool iterator_has_next(Iterator i) {
    return (i->step < i->size);
}

static inline void deleteIterator(Iterator *i) {
    free(*i);
    *i = nullptr;
}

static inline void iterator_advance(Iterator *i, IteratorNext next, size_t esize) {
    (*i)->step++;

    if (iterator_has_next(*i)) (*i)->ref = next(*i, esize);
    else deleteIterator(i);
}

#endif

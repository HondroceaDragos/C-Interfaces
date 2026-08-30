#ifndef BUCKET_H
#define BUCKET_H

#include "./helpers.h"
#include "./node.h"
#include "./raise.h"

typedef struct _bucket {
    NodeLink *head;
    NodeLink *tail;
    size_t size;
} Bucket;

static inline Bucket *newBucket(void) {
    Bucket *b = calloc(1, sizeof(Bucket));
    if (!b) raise(ERROR, "Cannot " RED "create bucket" RESET " (out-of memory).");
}

static inline void bucketPush(Bucket *b, NodeLink *n) {
    if (!b->tail) b->head = b->tail = n;
    else {
        b->tail->next = n;
        b->tail = n;
    }
    b->size++;
}

#endif

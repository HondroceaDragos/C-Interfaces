#ifndef VTABLE_LIST_H
#define VTABLE_LIST_H

#include "./helpers.h"
#include "./raise.h"
#include "./node.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#define ListVTableFunctions(id) \
    NodeLink *(*at)(id, int32_t); \
    struct { \
        void (*front)(id, NodeLink *); \
        void (*rear)(id, NodeLink *); \
    } push; \
    struct { \
        NodeLink *(*front)(id); \
        NodeLink *(*rear)(id); \
    } pop; \
    void (*reverse)(id); \
    int8_t *(*toString)(id); \
    void (*insert)(id, NodeLink *, int32_t); \
    bool (*empty)(id); \
    void (*clear)(id); \
    bool (*reachable)(id); \
    NodeLink *(*remove)(id, int32_t);

#define ListVTableType(id) \
    typedef struct _list_vtable_ { \
        ListVTableFunctions(id) \
    } LinkedListVTable; \
    static inline void delete(id)(id *l) { \
        if (!l || !*l) return; \
        NodeLink *iter = (*l)->head; \
        free(*l); \
        *l = nullptr; \
    } \
    \
    static inline NodeLink *_list_default_at(LinkedList self, int32_t idx) { \
        int32_t size = (int32_t)self->size; \
        if (!self) raise(ERROR, "Cannot index an " RED "empty container " RESET "(nil)."); \
        if (!size) { \
            raise(WARNING, "List is " YELLOW "empty " RESET "(size = %zu).", self->size); \
            raise(WARNING, "Returning " YELLOW "invalid reference " RESET "(nullptr)."); \
            return nullptr; \
        } \
        if (idx >= size) { \
            raise(WARNING, "Index " YELLOW "out-of-bounds" RESET ". idx = %d is not in range [0, %d].", idx, size); \
            raise(WARNING, "Returning " YELLOW "invalid reference " RESET "(nullptr)."); \
            return nullptr; \
        } \
        if (idx < -size) { \
            raise(WARNING, "Index " YELLOW "out-of-bounds" RESET ". idx = %d is not in range [%d, -1].", idx, -size); \
            raise(WARNING, "Returning " YELLOW "invalid reference " RESET "(nullptr)."); \
            return nullptr; \
        } \
        idx = (idx < 0) ? (size + idx) : idx; \
        NodeLink *iter = self->head; \
        int32_t i = 0; \
        while (i < idx && iter) { \
            iter = iter->next; \
            i++; \
        } \
        \
        return iter; \
    } \
    static inline void _list_default_push_rear(LinkedList self, NodeLink *nl) { \
        if (!self) raise(ERROR, "Cannot push into an " RED "empty container " RESET "(nil)."); \
        if (!nl) { \
            raise(WARNING, "Cannot " YELLOW "push value " RESET "(of-type: nil). List elements will remain unchanged."); \
            return; \
        } \
        if (!self->tail) { \
            nl->next = nl; \
            nl->prev = nl; \
            self->tail = nl; \
            self->head = nl; \
            self->size = 1; \
            return; \
        } \
        nl->next = self->head; \
        nl->prev = self->tail; \
        \
        self->tail->next = nl; \
        self->head->prev = nl; \
        \
        self->tail = self->tail->next; \
        self->size++; \
    } \
    static inline void _list_default_push_front(LinkedList self, NodeLink *nl) { \
        if (!self) { \
            raise(ERROR, "Cannot push into an " RED "empty container " RESET "(nil)."); \
        } \
        if (!nl) { \
            raise(WARNING, "Cannot " YELLOW "push value " RESET "(of-type: nil). List elements will remain unchanged."); \
            return; \
        } \
        if (!self->head) { \
            nl->next = nl; \
            nl->prev = nl; \
            self->head = nl; \
            self->tail = nl; \
            self->size = 1; \
            return; \
        } \
        nl->prev = self->tail; \
        nl->next = self->head; \
        \
        self->head->prev = nl; \
        self->tail->next = nl; \
        \
        self->head = nl; \
        self->size++; \
    } \
    static inline NodeLink *_list_default_pop_rear(LinkedList self) { \
        if (!self) { \
            raise(ERROR, "Cannot pop an " RED "empty container " RESET "(nil)."); \
        } \
        if (!self->tail) { \
            raise(WARNING, "List is " YELLOW "empty " RESET "(size = %zu).", self->size); \
            raise(WARNING, "Returning " YELLOW "invalid reference " RESET "(nullptr)."); \
            return nullptr; \
        } \
        NodeLink *ret = self->tail; \
        if (self->size == 1) self->head = self->tail = nullptr; \
        else { \
            self->tail = self->tail->prev; \
            self->tail->next = self->head; \
            self->head->prev = self->tail; \
        } \
        \
        ret->prev = ret->next = nullptr; \
        self->size--; \
        \
        return ret; \
    } \
    static inline NodeLink *_list_default_pop_front(LinkedList self) { \
        if (!self) { \
            raise(ERROR, "Cannot pop an " RED "empty container " RESET "(nil)."); \
        } \
        if (!self->head) { \
            raise(WARNING, "List is " YELLOW "empty " RESET "(size = %zu).", self->size); \
            raise(WARNING, "Returning " YELLOW "invalid reference " RESET "(nullptr)."); \
            return nullptr; \
        } \
        NodeLink *ret = self->head; \
        if (self->size == 1) self->head = self->tail = nullptr; \
        else { \
            self->head = self->head->next; \
            self->head->prev = self->tail; \
            self->tail->next = self->head; \
        } \
        \
        ret->prev = ret->next = nullptr; \
        self->size--; \
        \
        return ret; \
    } \
    static inline void _list_default_reverse(LinkedList self) { \
        if (!self) raise(ERROR, "Cannot reverse an " RED "empty container " RESET "(nil)."); \
        if (!self->head) raise(WARNING, "List is " YELLOW "empty " RESET "(size = %zu).", self->size); \
        \
        NodeLink *iter = self->tail; \
        size_t idx = 0; \
        while (idx < self->size && iter) { \
            NodeLink *tmp = iter->prev; \
            iter->prev = iter->next; \
            iter->next = tmp; \
            idx++; \
            iter = tmp; \
        } \
        \
        iter = self->head; \
        self->head = self->tail; \
        self->tail = iter; \
    } \
    static inline int8_t *_list_default_tostring(id self) { \
        if (!self) raise(ERROR, "Cannot format an " RED "empty container " RESET "(nil)."); \
        size_t bsize = 0; \
        size_t bcap = 2048; \
        \
        int8_t *buffer = calloc(bcap, sizeof(*buffer)); \
        buffer[0] = '\0'; \
        \
        int32_t slen = snprintf(buffer, bcap, "List(%zu) {\n\t(Addr: %p) <-\n[head]:", self->size, self->head->prev); \
        bsize += slen; \
        size_t size = self->size; \
        \
        NodeLink *iter = self->head; \
        for (size_t idx = 0; idx < size; idx++) { \
            int8_t *element = calloc(256, sizeof(*element)); \
            if (!element) { \
                raise(ERROR, "Die"); \
            } \
            (idx != size - 1) ? snprintf(element, 256, "\t(Addr: %p)", iter) : snprintf(element, 256, "[tail]: (Addr: %p)", iter); \
            size_t elen = strlen(element); \
            \
            const int8_t *sep = (idx + 1 < size) ? " <->\n" : " ->\n\t"; \
            size_t seplen = strlen(sep); \
            \
            size_t space = bsize + elen + seplen + 1; \
            \
            if (space > bcap) { \
                size_t new_cap = bcap; \
                while (space > new_cap) new_cap *= 2; \
                int8_t *tmp = buffer; \
                tmp = realloc(buffer, new_cap * sizeof(*buffer)); \
                if (!tmp) { \
                    raise(ERROR, "Cannot build list string " RED "(out-of-memory)" RESET ". List elements will remain unchanged."); \
                    free(element); \
                    free(buffer); \
                    return strdup("[?]"); \
                } \
                buffer = tmp; \
                bcap = new_cap; \
            } \
            \
            bsize += snprintf(buffer + bsize, bcap - bsize, "%s%s", element, sep); \
            free(element); \
            iter = iter->next; \
        } \
        \
        snprintf(buffer + bsize, bcap - bsize, "(Addr: %p)\n}", self->tail->next); \
        return buffer; \
    } \
    static inline void _list_default_insert(id self, NodeLink *value, int32_t idx) { \
        if (!self) raise(ERROR, "Cannot insert into an " RED "empty container " RESET "(nil)."); \
        int32_t size = (int32_t)self->size; \
        if (!size || !self->head) { \
            value->next = value; \
            value->prev = value; \
            self->head = value; \
            self->tail = value; \
            self->size = 1; \
            return; \
        } \
        if (idx > size) { \
            raise(WARNING, "Index " YELLOW "out-of-bounds" RESET ". idx = %d is not in range [0, %d].", idx, size); \
            raise(WARNING, "List elements will remain unchanged."); \
            return; \
        } \
        if (idx < -size) { \
            raise(WARNING, "Index " YELLOW "out-of-bounds" RESET ". idx = %d is not in range [0, %d].", idx, size); \
            raise(WARNING, "List elements will remain unchanged."); \
            return; \
        } \
        \
        NodeLink *prev = nullptr; \
        NodeLink *next = nullptr; \
        \
        if (idx == size) { \
            prev = self->tail; \
            next = self->head; \
            self->tail = value; \
        } else if (idx == 0 || idx == -size) { \
            prev = self->tail; \
            next = self->head; \
            self->head = value; \
        } else if (idx > 0) { \
            next = self->head; \
            for (int32_t i = 0; i < idx; i++) next = next->next; \
            prev = next->prev; \
        } else { \
            next = self->tail; \
            for (int32_t i = -1; i > idx; i--) next = next->prev; \
            prev = next->prev; \
        } \
        \
        value->prev = prev; \
        value->next = next; \
        \
        prev->next = value; \
        next->prev = value; \
        \
        self->size++; \
    } \
    static inline bool _list_default_empty(id self) { \
        if (!self) raise(ERROR, "Cannot check size of an " RED "empty container " RESET "(nil)."); \
        return (self->size == 0); \
    } \
    static inline void _list_default_clear(id self) { \
        if (!self) raise(ERROR, "Cannot clear an " RED "empty container " RESET "(nil)."); \
        self->head = nullptr; \
        self->tail = nullptr; \
        self->size = 0; \
    } \
    static inline bool _list_default_reachable(id self) { \
        return (self->head || self->tail); \
    } \
    static inline NodeLink *_list_default_remove(id self, int32_t idx) { \
        int32_t size = (int32_t)self->size; \
        if (!self) raise(ERROR, "Cannot remove from an " RED "empty container " RESET "(nil)."); \
        if (!size || !self->head) { \
            raise(WARNING, "Cannot " YELLOW "remove " RESET "from an " YELLOW "empty list" RESET "."); \
            raise(WARNING, "Returning " YELLOW "invalid reference " RESET "(nullptr)."); \
            return nullptr; \
        } \
        if (idx >= size) { \
            raise(WARNING, "Index " YELLOW "out-of-bounds" RESET ". idx = %d is not in range [0, %d].", idx, size - 1); \
            raise(WARNING, "Returning " YELLOW "invalid reference " RESET "(nullptr). List elements will remain unchanged."); \
            return nullptr; \
        } \
        if (idx < -size) { \
            raise(WARNING, "Index " YELLOW "out-of-bounds" RESET ". idx = %d is not in range [0, %d].", idx, size); \
            raise(WARNING, "Returning " YELLOW "invalid reference " RESET "(nullptr). List elements will remain unchanged."); \
            return nullptr; \
        } \
        \
        NodeLink *prev = nullptr; \
        NodeLink *next = nullptr; \
        NodeLink *ret = nullptr; \
        \
        if (idx == size - 1) ret = self->tail; \
        else if (idx >= 0) { \
            ret = self->head; \
            for (int32_t i = 0; i < idx; i++) ret = ret->next; \
        } else { \
            ret = self->tail; \
            for (int32_t i = -1; i > idx; i--) ret = ret->prev; \
        } \
        \
        prev = ret->prev; \
        next = ret->next; \
        \
        if (self->size == 1) self->head = self->tail = nullptr; \
        else { \
            prev->next = next; \
            next->prev = prev; \
            \
            if (ret == self->head) self->head = next; \
            if (ret == self->tail) self->tail = prev; \
        } \
        \
        ret->next = ret->prev = nullptr; \
        \
        self->size--; \
        return ret; \
    } \
    static LinkedListVTable LinkedListVTableInstance = { \
        .at = _list_default_at, \
        .push.rear = _list_default_push_rear, \
        .push.front = _list_default_push_front, \
        .pop.rear = _list_default_pop_rear, \
        .pop.front = _list_default_pop_front, \
        .reverse = _list_default_reverse, \
        .toString = _list_default_tostring, \
        .insert = _list_default_insert, \
        .empty = _list_default_empty, \
        .clear = _list_default_clear, \
        .reachable = _list_default_reachable, \
        .remove = _list_default_remove, \
    }; \

#endif

#ifndef VTABLE_STRING_H
#define VTABLE_STRING_H

#include "./helpers.h"
#include "./raise.h"

#define StringVTableFunctions(id) \
    id (*reverse)(id); \
    struct { \
        id (*c_str)(id, const int8_t *); \
        id (*str)(id, str); \
        id (*string)(id, id); \
    } concat;

#define StringVTableType(id) \
    typedef struct _string_vtable_ { \
        StringVTableFunctions(id) \
    } StringVTable; \
    \
    static inline void delete(id)(id *s) { \
        if (!s || !*s) return; \
        free((void *)(*s)->data); \
        free(*s); \
        *s = nullptr; \
    } \
    \
    static inline id _string_default_reverse(id self) { \
        int8_t *rev = strdup(self->data); \
        if (!rev) raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory)."); \
        \
        size_t size = self->size; \
        for (size_t idx = 0; idx < size / 2; idx++) { \
            swap(rev[idx], rev[size - idx - 1]); \
        } \
        \
        id ret = calloc(1, sizeof(*ret)); \
        if (!ret) raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory)."); \
        *ret = *self; \
        ret->data = rev; \
        return ret; \
    } \
    \
    static inline id _string_default_concat_primitive(id self, const int8_t *other, size_t len) { \
        size_t size = self->size + len; \
        int8_t *s = calloc(size + 1, sizeof(*s)); \
        if (!s) raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory)."); \
        \
        memcpy(s, self->data, self->size); \
        memcpy(s + self->size, other, len); \
        s[size] = '\0'; \
        \
        id ret = calloc(1, sizeof(*ret)); \
        if (!ret) raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory)."); \
        *ret = *self; \
        ret->data = s; \
        ret->size = size; \
        return ret; \
    } \
    static inline id _string_default_concat_c_str(id self, const int8_t *other) { \
        return _string_default_concat_primitive(self, other, strlen(other)); \
    } \
    static inline id _string_default_concat_str(id self, str other) { \
        return _string_default_concat_primitive(self, other.data, other.size); \
    } \
    static inline id _string_default_concat_string(id self, id other) { \
        return _string_default_concat_primitive(self, other->data, other->size); \
    } \
    \
    static StringVTable StringVTableInstance = { \
            .reverse = _string_default_reverse, \
            .concat.c_str = _string_default_concat_c_str, \
            .concat.str = _string_default_concat_str, \
            .concat.string = _string_default_concat_string, \
        }; \

#endif

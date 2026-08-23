#ifndef STRING_H
#define STRING_H

#include "./raise.h"
#include "./helpers.h"
#include "./vtable_string.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef struct _str {
    const int8_t *data;
    size_t size;
} str;

str newStr(const int8_t *c_str) {
    return (str){
        .data = c_str,
        .size = strlen((const char *)c_str)
    };
}

typedef struct _string {
    const int8_t *data;
    size_t size;

    /**
     * reverse() [DONE]
     * chop() / slide()
     * tokenize()
     * sort()
     * filter()?
     * insert()
     * remove()
     * concat() [DONE]
     * find()
     * trim()
     * toUpper()
     * toLower()
     * contains()?
     * join()
     * constructor from fmt? (use snprintf)
     * repeat()
     * pad() ?
     */
    StringVTableFunctions(struct _string *);
} *String;

StringVTableType(String)

String newStringPrimitive(const int8_t *s, size_t slen) {
    String ret = (String)calloc(1, sizeof(*ret));
    if (!ret) {
        raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory).");
    }

    ret->data = (int8_t *)strdup((const char *)s);
    if (!ret->data) {
        free(ret);
        raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory).");
    }

    ret->size = slen;

    ret->reverse = StringVTableInstance.reverse;

    ret->concat.c_str = StringVTableInstance.concat.c_str;
    ret->concat.str = StringVTableInstance.concat.str;
    ret->concat.string = StringVTableInstance.concat.string;

    return ret;
}

String newStringFromCStr(const int8_t *s) {
    return newStringPrimitive(s, strlen(s));
}

String newStringFromStr(str s) {
    return newStringPrimitive(s.data, s.size);
}

String newStringFromString(String s) {
    return newStringPrimitive(s->data, s->size);
}

#define newString(s) _Generic((s), \
    String: newStringFromString, \
    str: newStringFromStr, \
    default: newStringFromCStr \
)(s)

#endif

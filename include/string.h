#ifndef STRING_H
#define STRING_H

#include "./raise.h"
#include "./helpers.h"
#include "./vtable_string.h"
#include "./vtable_string_builder.h"
#include "./iterator.h"
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

static inline void *_string_next(Iterator i, size_t esize) {
    return (int8_t *)i->ref + esize;
}

typedef struct _string {
    const int8_t *data;
    size_t size;
    StringVTableFunctions(struct _string *);
} *String;

StringVTableType(String)

String newStringPrimitive(const int8_t *s, size_t slen) {
    String ret = (String)calloc(1, sizeof(*ret));
    if (!ret) {
        raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory).");
    }

    i8 *data = calloc(slen + 1, sizeof(*data));
    if (!data) raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory).");
    memcpy(data, s, slen);
    data[slen] = '\0';

    ret->data = data;
    ret->size = slen;

    ret->reverse = StringVTableInstance.reverse;

    ret->concat.c_str = StringVTableInstance.concat.c_str;
    ret->concat.str = StringVTableInstance.concat.str;
    ret->concat.string = StringVTableInstance.concat.string;

    ret->chop.left = StringVTableInstance.chop.left;
    ret->chop.right = StringVTableInstance.chop.right;

    ret->find.ch = StringVTableInstance.find.ch;
    ret->find.c_str = StringVTableInstance.find.c_str;
    ret->find.str = StringVTableInstance.find.str;
    ret->find.string = StringVTableInstance.find.string;

    ret->substring = StringVTableInstance.substring;

    ret->trim.left = StringVTableInstance.trim.left;
    ret->trim.right = StringVTableInstance.trim.right;
    ret->trim.both = StringVTableInstance.trim.both;

    ret->tokenize.c_str = StringVTableInstance.tokenize.c_str;
    ret->tokenize.str = StringVTableInstance.tokenize.str;
    ret->tokenize.string = StringVTableInstance.tokenize.string;

    ret->map =  StringVTableInstance.map;

    ret->cmp.c_str = StringVTableInstance.cmp.c_str;
    ret->cmp.str = StringVTableInstance.cmp.str;
    ret->cmp.string = StringVTableInstance.cmp.string;

    ret->sort = StringVTableInstance.sort;

    return ret;
}

String newStringFromCStr(const int8_t *s) {
    const int8_t *data = (s) ? s : (const int8_t *)"";
    return newStringPrimitive(data, strlen(data));
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

String newStringFmt(const i8* fmt, ...) {
    va_list args;

    va_start(args, fmt);

    i32 slen = vsnprintf(nullptr, 0, fmt, args);

    if (slen < 0) {
        va_end(args);
        raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory).");
    }

    slen += 1;
    i8 *data = calloc(slen, sizeof(*data));
    if (!data) {
        va_end(args);
        raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory).");
    }

    vsnprintf(data, slen, fmt, args);

    return newStringPrimitive(data, slen);
}

str newStrFromString(String s, i32 start, i32 end) {
    size_t size = s->size;

    if (start < 0 || end < 0) {
        raise(WARNING, "Invalid " YELLOW "interval" RESET ", not in range [0, %zu).", size); \
        raise(WARNING, "Returning empty str."); \
        return (str) {
            .data = "",
            .size = 0
        };
    }

    if (start < 0) {
        raise(WARNING, "Start index " YELLOW "out-of-bounds" RESET ". start = %d is not in range [0, %zu).", start, size); \
        raise(WARNING, "Returning empty str."); \
        return (str) {
            .data = "",
            .size = 0
        };
    }
    if (end >= size) {
        raise(WARNING, "End index " YELLOW "out-of-bounds" RESET ". end = %d is not in range [0, %zu).", end, size); \
        raise(WARNING, "Returning empty str."); \
        return (str) {
            .data = "",
            .size = 0
        };
    }
    if (start > end) {
        raise(WARNING, "Invalid " YELLOW "str interval." RESET " start = %d > end = %d.", start, end); \
        raise(WARNING, "Returning empty str."); \
        return (str) {
            .data = "",
            .size = 0
        };
    }

    return (str) {
        .data = s->data + start,
        .size = (size_t)(end - start + 1)
    };
}

typedef struct _string_builder {
    i8 *data;
    size_t size;

    StringBuilderVTableFunctions(struct _string_builder *)
} *StringBuilder;

StringBuilderVTableType(StringBuilder)

StringBuilder newStringBuilderPrimitive(const int8_t *s, size_t slen) {
    StringBuilder ret = (StringBuilder)calloc(1, sizeof(*ret));
    if (!ret) {
        raise(ERROR, "Cannot " RED "create StringBuilder " RESET "(out-of-memory).");
    }

    i8 *data = calloc(slen + 1, sizeof(*data));
    if (!data) raise(ERROR, "Cannot " RED "create StringBuilder " RESET "(out-of-memory).");
    memcpy(data, s, slen);
    data[slen] = '\0';

    ret->data = data;
    ret->size = slen;

    ret->reverse = StringBuilderVTableInstance.reverse;

    ret->concat.c_str = StringBuilderVTableInstance.concat.c_str;
    ret->concat.str = StringBuilderVTableInstance.concat.str;
    ret->concat.string = StringBuilderVTableInstance.concat.string;

    ret->chop.left = StringBuilderVTableInstance.chop.left;
    ret->chop.right = StringBuilderVTableInstance.chop.right;

    ret->substring = StringBuilderVTableInstance.substring;

    ret->trim.left = StringBuilderVTableInstance.trim.left;
    ret->trim.right = StringBuilderVTableInstance.trim.right;
    ret->trim.both = StringBuilderVTableInstance.trim.both;

    ret->map =  StringBuilderVTableInstance.map;

    ret->sort = StringBuilderVTableInstance.sort;

    ret->consume = StringBuilderVTableInstance.consume;

    return ret;
}

StringBuilder newStringBuilderFromCStr(const int8_t *s) {
    const int8_t *data = (s) ? s : (const int8_t *)"";
    return newStringBuilderPrimitive(data, strlen(data));
}

StringBuilder newStringBuilderFromStr(str s) {
    return newStringBuilderPrimitive(s.data, s.size);
}

StringBuilder newStringBuilderFromString(String s) {
    return newStringBuilderPrimitive(s->data, s->size);
}

#define newStringBuilder(s) _Generic((s), \
    String: newStringBuilderFromString, \
    str: newStringBuilderFromStr, \
    default: newStringBuilderFromCStr \
)(s)

#define forString_primitive(i, once, str, acc) \
    for (Iterator i = newIterator((str)->data, (str)->size); i && i->size; iterator_advance(&i, _string_next, 1)) \
        for (acc = (int8_t *)i->ref, *once = (void *)1; once; once = 0)

#define forString(acc, str) \
    forString_primitive(concat_layer2(_i_, __COUNTER__), concat_layer2(_once_, __COUNTER__), str, acc)

#endif

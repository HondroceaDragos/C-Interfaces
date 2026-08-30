#ifndef VTABLE_STRING_H
#define VTABLE_STRING_H

#include "./helpers.h"
#include "./raise.h"

typedef i8 (*funcMapCh)(i8);

#define StringVTableFunctions(id) \
    id (*reverse)(id); \
    struct { \
        id (*c_str)(id, const int8_t *); \
        id (*str)(id, str); \
        id (*string)(id, id); \
    } concat; \
    struct { \
        id (*left)(id, size_t); \
        id (*right)(id, size_t); \
    } chop; \
    struct { \
        i32 (*ch)(id, i8); \
        i32 (*c_str)(id, const i8 *); \
        i32 (*str)(id, str); \
        i32 (*string)(id, id); \
    } find; \
    id (*substring)(id, i32, i32); \
    struct { \
        id (*left)(id); \
        id (*right)(id); \
        id (*both)(id); \
    } trim; \
    struct { \
        id *(*c_str)(id, const i8 *); \
        id *(*str)(id, str); \
        id *(*string)(id, id); \
    } tokenize; \
    id (*map)(id, funcMapCh); \
    struct { \
        bool (*c_str)(id, const i8 *); \
        bool (*str)(id, str); \
        bool (*string)(id, id); \
    } cmp; \
    id (*sort)(id, i32 (*cmp)(const void *, const void *)); \
    i8 *(*toString)(id);

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
    static inline id _string_default_chop_primitive(id self, size_t count, size_t offset) { \
        id ret = calloc(1, sizeof(*ret)); \
        if (!ret) raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory)."); \
        *ret = *self; \
        \
        if (count > self->size) { \
            raise(WARNING, "Count " YELLOW "exceeds string length " RESET "(%zu >= %zu). Retunring empty string.", count, self->size); \
            ret->data = strdup(""); \
            ret->size = 0; \
            return ret; \
        } \
        size_t size = self->size - count; \
        \
        int8_t *s = calloc(size + 1, sizeof(*s)); \
        if (!s) raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory)."); \
        \
        memcpy(s, self->data + offset, size); \
        s[size] = '\0'; \
        \
        ret->data = s; \
        ret->size = size; \
        return ret; \
    } \
    static inline id _string_default_chop_left(id self, size_t count) { \
        return _string_default_chop_primitive(self, count, count); \
    } \
    static inline id _string_default_chop_right(id self, size_t count) { \
        return _string_default_chop_primitive(self, count, 0); \
    } \
    static inline i32 _string_default_find_ch(id self, i8 ch) { \
        for (size_t idx = 0; idx < self->size; idx++) { \
            if (self->data[idx] == ch) return idx; \
        } \
        return notfound; \
    } \
    static inline i32 _string_default_find_primitive(id self, const i8 *text) { \
        i8 *start = strstr(self->data, text); \
        if (start) return start - self->data; \
        return notfound; \
    } \
    static inline i32 _string_default_find_str(id self, str s) { \
        return _string_default_find_primitive(self, s.data); \
    } \
    static inline i32 _string_default_find_string(id self, id s) { \
        return _string_default_find_primitive(self, s->data); \
    } \
    static inline id _string_default_substring(id self, i32 start, i32 end) { \
        size_t size = self->size; \
        id ret = calloc(1, sizeof(*ret)); \
        if (!ret) raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory)."); \
        *ret = *self; \
        \
        if (start < 0 || start >= (i32)size) { \
            raise(WARNING, "Start index " YELLOW "out-of-bounds" RESET ". start = %d is not in range [0, %zu).", start, size); \
            raise(WARNING, "Returning empty string."); \
            ret->data = strdup(""); \
            ret->size = 0; \
            return ret; \
        } \
        \
        if (end >= (i32)size || end < 0) { \
            raise(WARNING, "End index " YELLOW "out-of-bounds" RESET ". end = %d is not in range [0, %zu).", end, size); \
            raise(WARNING, "Returning empty string."); \
            ret->data = strdup(""); \
            ret->size = 0; \
            return ret; \
        } \
        \
        bool shouldReverse = (start > end); \
        \
        i32 mini = start; \
        i32 maxi = end; \
        if (start > end) { \
            mini = end; \
            maxi = start; \
        } \
        size = (size_t)(maxi - mini + 1); \
        \
        i8 *data = calloc(size + 1, sizeof(*data)); \
        if (!data) raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory)."); \
        \
        memcpy(data, self->data + (size_t)mini, size); \
        data[size] = '\0'; \
        \
        ret->data = data; \
        ret->size = size; \
        \
        if (shouldReverse) { \
            for (size_t idx = 0; idx < size / 2; idx++) { \
                swap(data[idx], data[size - idx - 1]); \
            } \
        } \
        \
        return ret; \
    } \
    static inline id _string_trim_primitive(id self, size_t offset) { \
        size_t size = self->size; \
        id ret = calloc(1, sizeof(*ret)); \
        if (!ret) raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory)."); \
        *ret = *self; \
        \
        size_t start = 0; \
        if (offset & 1) while (start < size && isspace(self->data[start])) start++; \
        \
        size_t end = self->size; \
        if (offset & 2) while (end > start && isspace(self->data[end - 1])) end--; \
        \
        size = end - start; \
        i8 *data = calloc(size + 1, sizeof(*data)); \
        if (!data) raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory)."); \
        memcpy(data, self->data + start, size); \
        \
        ret->data = data; \
        ret->size = size; \
        \
        return ret; \
    } \
    static inline id _string_default_trim_left(id self) { \
        _string_trim_primitive(self, 1); \
    } \
    static inline id _string_default_trim_right(id self) { \
        _string_trim_primitive(self, 2); \
    } \
    static inline id _string_default_trim_both(id self) { \
        _string_trim_primitive(self, 3); \
    } \
    static inline id *_string_tokenize_primitive(id self, const i8 *data, size_t len) { \
        size_t size = self->size; \
        \
        String *tokens = calloc((size + 1) / 2 + 1, sizeof(*tokens)); \
        if (!tokens) raise(ERROR, "Whoop"); \
        \
        str token = {.data = self->data, .size = 0}; \
        size_t tidx = 0; \
        \
        for (size_t idx = 0; idx <= size; idx++) { \
            size_t jdx = 0; \
            while (jdx < size && jdx <len && self->data[idx] != data[jdx]) jdx++; \
            \
            if ((idx == size || jdx < len) && token.size) { \
                id slot = calloc(1, sizeof(*slot)); \
                if (!slot) raise(ERROR, "Whoop"); \
                \
                *slot = *self; \
                \
                i8 *s = calloc(token.size + 1, sizeof(*slot->data)); \
                if (!s) raise(ERROR, "Whoop"); \
                memcpy(s, token.data, token.size); \
                s[token.size] = '\0'; \
                \
                slot->data = s; \
                slot->size = token.size; \
                \
                tokens[tidx++] = slot; \
                \
                token.data = self->data + (idx + 1); \
                token.size = 0; \
            } else token.size++; \
        } \
        \
        tokens[tidx] = nullptr; \
        \
        return tokens; \
    } \
    static inline id *_string_default_tokenize_c_str(id self, const i8 *data) { \
        return _string_tokenize_primitive(self, data, strlen(data)); \
    } \
    static inline id *_string_default_tokenize_str(id self, str data) { \
        return _string_tokenize_primitive(self, data.data, data.size); \
    } \
    static inline id *_string_default_tokenize_string(id self, id data) { \
        return _string_tokenize_primitive(self, data->data, data->size); \
    } \
    static inline id _string_default_map(id self, funcMapCh map) { \
        size_t size = self->size; \
        \
        id ret = calloc(1, sizeof(*ret)); \
        if (!ret) raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory)."); \
        *ret = *self; \
        \
        i8 *data = strdup(self->data); \
        if (!data) raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory)."); \
        for (size_t idx = 0; idx < size; idx++) if (map) data[idx] = map(data[idx]); \
        \
        ret->data = data; \
        ret->size = size; \
        \
        return ret; \
    } \
    static inline i32 _string_qsort_cmp(const void *a, const void *b) { \
        const i8 *sta = *(const i8 **)a; \
        const i8 *stb = *(const i8 **)b; \
        \
        size_t stalen = strlen(sta); \
        size_t stblen = strlen(stb); \
        \
        if (stalen < stblen) return -1; \
        else if (stalen > stblen) return 1; \
        \
        return strncmp(sta, stb, stalen); \
    } \
    static inline bool _string_cmp_primitive(id self, const i8 *data) { \
        i32 ret = _string_qsort_cmp(&self->data, &data); \
        \
        if ((ret < 0) || (ret > 0)) return false; \
        return true; \
    } \
    static inline bool _string_default_cmp_c_str(id self, const i8 *data) { \
        return _string_cmp_primitive(self, data); \
    } \
    static inline bool _string_default_cmp_str(id self, str data) { \
        return _string_cmp_primitive(self, data.data); \
    } \
    static inline bool _string_default_cmp_string(id self, id data) { \
        return _string_cmp_primitive(self, data->data); \
    } \
    static inline i32 chcmp(const void *a, const void *b) { \
        i8 ca = *(i8 *)a; \
        i8 cb = *(i8 *)b; \
        \
        return ca - cb; \
    } \
    static inline id _string_default_sort(id self, i32 (*cmp)(const void *, const void *)) { \
        size_t size = self->size; \
        \
        id ret = calloc(1, sizeof(*ret)); \
        if (!ret) raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory)."); \
        *ret = *self; \
        \
        i8 *data = strdup(self->data); \
        if (!data) raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory)."); \
        \
        qsort(data, size, sizeof(i8), cmp); \
        \
        ret->data = data; \
        ret->size = size; \
        \
        return ret; \
    } \
    static inline i8 *_string_default_toString(id self) { \
        return strdup(self->data); \
    } \
    \
    static StringVTable StringVTableInstance = { \
            .reverse = _string_default_reverse, \
            .concat.c_str = _string_default_concat_c_str, \
            .concat.str = _string_default_concat_str, \
            .concat.string = _string_default_concat_string, \
            .chop.left = _string_default_chop_left, \
            .chop.right = _string_default_chop_right, \
            .find.ch = _string_default_find_ch, \
            .find.c_str = _string_default_find_primitive, \
            .find.str = _string_default_find_str, \
            .find.string = _string_default_find_string, \
            .substring = _string_default_substring, \
            .trim.left = _string_default_trim_left, \
            .trim.right = _string_default_trim_right, \
            .trim.both = _string_default_trim_both, \
            .tokenize.c_str = _string_default_tokenize_c_str, \
            .tokenize.str = _string_default_tokenize_str, \
            .tokenize.string = _string_default_tokenize_string, \
            .map = _string_default_map, \
            .cmp.c_str = _string_default_cmp_c_str, \
            .cmp.str = _string_default_cmp_str, \
            .cmp.string = _string_default_cmp_string, \
            .sort = _string_default_sort, \
            .toString = _string_default_toString, \
        }; \

#endif

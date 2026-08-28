#ifndef VTABLE_STRING_BUILDER_H
#define VTABLE_STRING_BUILDER_H

#include "./helpers.h"
#include "./raise.h"

#define StringBuilderVTableFunctions(id) \
    id (*reverse)(id); \
    struct { \
        id (*c_str)(id, const int8_t *); \
        id (*str)(id, str); \
        id (*string)(id, String); \
    } concat; \
    struct { \
        id (*left)(id, size_t); \
        id (*right)(id, size_t); \
    } chop; \
    id (*substring)(id, i32, i32); \
    struct { \
        id (*left)(id); \
        id (*right)(id); \
        id (*both)(id); \
    } trim; \
    id (*map)(id, funcMapCh); \
    id (*sort)(id, i32 (*cmp)(const void *, const void *)); \
    String (*consume)(id *); \
    i8 *(*release)(id *);

#define StringBuilderVTableType(id) \
    typedef struct _string_builder_vtable_ { \
        StringBuilderVTableFunctions(id) \
    } StringBuilderVTable; \
    \
    static inline id _string_builder_default_reverse(id self) { \
        if (!self || !self->data) { \
            raise(ERROR, " RED ""Invalid reference " RESET "used for StringBuilder. Failed at: " RED "reverse" RESET "."); \
            raise(ERROR, "Returning nullptr."); \
            return nullptr; \
        } \
        i8 *data = self->data; \
        size_t size = self->size; \
        \
        for (size_t idx = 0; idx < size / 2; idx++) { \
            swap(data[idx], data[size - idx - 1]); \
        } \
        \
        return self; \
    } \
    \
    static inline id _string_builder_default_concat_primitive(id self, const int8_t *other, size_t len) { \
        if (!self || !self->data) { \
            raise(ERROR, " RED ""Invalid reference " RESET "used for StringBuilder. Failed at: " RED "concat" RESET "."); \
            raise(ERROR, "Returning nullptr."); \
            return nullptr; \
        } \
        i8 *data = self->data; \
        size_t size = self->size + len; \
        \
        i8 *tmp = realloc(data, size + 1); \
        if (!tmp) raise(ERROR, "Cannot " RED "create string " RESET "(out-of-memory)."); \
        \
        data = tmp; \
        memcpy(data + self->size, other, len); \
        data[size] = '\0'; \
        \
        self->data = data; \
        self->size = size; \
        \
        return self; \
    } \
    static inline id _string_builder_default_concat_c_str(id self, const int8_t *other) { \
        return _string_builder_default_concat_primitive(self, other, strlen(other)); \
    } \
    static inline id _string_builder_default_concat_str(id self, str other) { \
        return _string_builder_default_concat_primitive(self, other.data, other.size); \
    } \
    static inline id _string_builder_default_concat_string(id self, String other) { \
        return _string_builder_default_concat_primitive(self, other->data, other->size); \
    } \
    static inline id _string_builder_default_chop_primitive(id self, size_t count, size_t offset) { \
        if (!self || !self->data) { \
            raise(ERROR, " RED ""Invalid reference " RESET "used for StringBuilder. Failed at: " RED "chop" RESET "."); \
            raise(ERROR, "Returning nullptr."); \
            return nullptr; \
        } \
        i8 *data = self->data; \
        size_t size = self->size; \
        \
        if (count > self->size) { \
            raise(WARNING, "Count " YELLOW "exceeds reference length " RESET "(%zu >= %zu). Failed at: " RED "chop" RESET ".", count, self->size); \
            raise(ERROR, "Returning nullptr."); \
            return nullptr; \
        } \
        size -= count; \
        \
        memcpy(data, data + offset, size); \
        data[size] = '\0'; \
        \
        self->size = size; \
        \
        return self; \
    } \
    static inline id _string_builder_default_chop_left(id self, size_t count) { \
        return _string_builder_default_chop_primitive(self, count, count); \
    } \
    static inline id _string_builder_default_chop_right(id self, size_t count) { \
        return _string_builder_default_chop_primitive(self, count, 0); \
    } \
    static inline id _string_builder_default_substring(id self, i32 start, i32 end) { \
        if (!self || !self->data) { \
            raise(ERROR, " RED ""Invalid reference " RESET "used for StringBuilder. Failed at: " RED "substring" RESET "."); \
            raise(ERROR, "Returning nullptr."); \
            return nullptr; \
        } \
        i8 *data = self->data; \
        size_t size = self->size; \
        \
        if (start < 0 || start >= (i32)size) { \
            raise(WARNING, "Start index " YELLOW "out-of-bounds" RESET ". start = %d is not in range [0, %zu). Failed at: " RED "substring" RESET ".", start, size); \
            raise(WARNING, "Returning nullptr."); \
            return nullptr; \
        } \
        \
        if (end >= (i32)size || end < 0) { \
            raise(WARNING, "End index " YELLOW "out-of-bounds" RESET ". end = %d is not in range [0, %zu). Failed at: " RED "substring" RESET ".", end, size); \
            raise(WARNING, "Returning nullptr."); \
            return nullptr; \
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
        \
        memcpy(data, data + (size_t)mini, size); \
        data[size] = '\0'; \
        \
        self->size = size; \
        \
        if (shouldReverse) { \
            for (size_t idx = 0; idx < size / 2; idx++) { \
                swap(data[idx], data[size - idx - 1]); \
            } \
        } \
        \
        return self; \
    } \
    static inline id _string_builder_trim_primitive(id self, size_t offset) { \
        if (!self || !self->data) { \
            raise(ERROR, " RED ""Invalid reference " RESET "used for StringBuilder. Failed at: " RED "trim" RESET "."); \
            raise(ERROR, "Returning nullptr."); \
            return nullptr; \
        } \
        i8 *data = self->data; \
        size_t size = self->size; \
        \
        size_t start = 0; \
        if (offset & 1) while (start < size && isspace(self->data[start])) start++; \
        \
        size_t end = self->size; \
        if (offset & 2) while (end > start && isspace(self->data[end - 1])) end--; \
        \
        size = end - start; \
        memcpy(data, data + start, size); \
        data[size] = '\0'; \
        \
        self->size = size; \
        \
        return self; \
    } \
    static inline id _string_builder_default_trim_left(id self) { \
        _string_builder_trim_primitive(self, 1); \
    } \
    static inline id _string_builder_default_trim_right(id self) { \
        _string_builder_trim_primitive(self, 2); \
    } \
    static inline id _string_builder_default_trim_both(id self) { \
        _string_builder_trim_primitive(self, 3); \
    } \
    static inline id _string_builder_default_map(id self, funcMapCh map) { \
        if (!self || !self->data) { \
            raise(ERROR, " RED ""Invalid reference " RESET "used for StringBuilder. Failed at: " RED "map" RESET "."); \
            raise(ERROR, "Returning nullptr."); \
            return nullptr; \
        } \
        i8 *data = self->data; \
        size_t size = self->size; \
        \
        for (size_t idx = 0; idx < size; idx++) if (map) data[idx] = map(data[idx]); \
        \
        return self; \
    } \
    static inline id _string_builder_default_sort(id self, i32 (*cmp)(const void *, const void *)) { \
        if (!self || !self->data) { \
            raise(ERROR, " RED ""Invalid reference " RESET "used for StringBuilder. Failed at: " RED "sort" RESET "."); \
            raise(ERROR, "Returning nullptr."); \
            return nullptr; \
        } \
        i8 *data = self->data; \
        size_t size = self->size; \
        \
        qsort(data, size, sizeof(i8), cmp); \
        \
        return self; \
    } \
    static inline String _string_builder_default_consume(id *self) { \
        String ret = newString((*self)->data); \
        free((*self)->data); \
        free(*self); \
        *self = nullptr; \
        \
        return ret; \
    } \
    static inline i8 *_string_builder_default_release(id *self) { \
        i8 *ret = (*self)->data; \
        free(*self); \
        *self = nullptr; \
        \
        return ret; \
    } \
    \
    static StringBuilderVTable StringBuilderVTableInstance = { \
            .reverse = _string_builder_default_reverse, \
            .concat.c_str = _string_builder_default_concat_c_str, \
            .concat.str = _string_builder_default_concat_str, \
            .concat.string = _string_builder_default_concat_string, \
            .chop.left = _string_builder_default_chop_left, \
            .chop.right = _string_builder_default_chop_right, \
            .substring = _string_builder_default_substring, \
            .trim.left = _string_builder_default_trim_left, \
            .trim.right = _string_builder_default_trim_right, \
            .trim.both = _string_builder_default_trim_both, \
            .map = _string_builder_default_map, \
            .sort = _string_builder_default_sort, \
            .consume = _string_builder_default_consume, \
            .release = _string_builder_default_release, \
        }; \

#endif

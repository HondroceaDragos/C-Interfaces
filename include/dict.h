#ifndef DICT_H
#define DICT_H

#include "./array.h"
#include "./vtable_dict.h"
#include "./raise.h"
#include "./iterator.h"
#include "./node.h"
#include "./pair.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DICT_DEFAULT_SIZE 16
#define DICT_DEFAULT_LOAD_FACTOR 0.75

#define DictType(type) \
    typedef struct concat_layer2(_dict_, type) { \
        Node(Pair(DictKey, type)) *data; \
        size_t size; \
        size_t capacity; \
        double loadFactor; \
        \
        DictVTableFunctions(struct concat_layer2(_dict_, type) *, type) \
    } *concat_layer2(Dict_, type); \
    \
    DictVTableType(concat_layer2(Dict_, type), type) \
    \
    struct concat_layer2(_dict_defaults_, type) { \
        Array(Pair(DictKey, type)) using; \
        size_t capacity; \
        size_t loadFactor; \
        DictVTableFunctions(concat_layer2(Dict_, type), type) \
    }; \
    \
    static inline concat_layer2(Dict_, type) concat_layer2(newDict_, type)(struct concat_layer2(_dict_defaults_, type) defaults) { \
        concat_layer2(Dict_, type) d = calloc(1, sizeof(*d)); \
        if (!d) { \
            raise(ERROR, "Cannot create dict " RED "(out-of-memory)" RESET ". Returning nullptr."); \
            return nullptr; \
        } \
        \
        d->hash = (defaults.hash) ? defaults.hash : DictVTableInstance(type).hash; \
        d->capacity = (defaults.capacity) ? defaults.capacity : DICT_DEFAULT_SIZE; \
        d->loadFactor = (defaults.loadFactor) ? defaults.loadFactor : DICT_DEFAULT_LOAD_FACTOR; \
        \
        d->data = calloc(d->capacity, sizeof(*d->data)); \
        if (!d->data) { \
            free(d); \
            raise(ERROR, "Cannot create dict " RED "(out-of-memory)" RESET ". Returning nullptr."); \
            return nullptr; \
        } \
        \
        d->fmt = (defaults.fmt) ? defaults.fmt : DictVTableInstance(type).fmt; \
        d->toString = (defaults.toString) ? defaults.toString : DictVTableInstance(type).toString; \
        \
        if (defaults.using.data) { \
            for (size_t idx = 0; idx < defaults.using.size; idx++) { \
                concat_layer2(_dict_default_put_, type)(d, defaults.using.data[idx]); \
            } \
        } \
        d->emplace = (defaults.emplace) ? defaults.emplace : DictVTableInstance(type).emplace; \
        d->get = (defaults.get) ? defaults.get : DictVTableInstance(type).get; \
        d->put = (defaults.put) ? defaults.put : DictVTableInstance(type).put; \
        \
        d->currLoad = (defaults.currLoad) ? defaults.currLoad : DictVTableInstance(type).currLoad; \
        d->contains = (defaults.contains) ? defaults.contains : DictVTableInstance(type).contains; \
        d->remove = (defaults.remove) ? defaults.remove : DictVTableInstance(type).remove; \
        \
        return d; \
    } \

#define Dict(type) concat_layer2(Dict_, type)
#define newDict(type, ...) \
    concat_layer2(newDict_, type)((struct concat_layer2(_dict_defaults_, type)){__VA_ARGS__})

#endif

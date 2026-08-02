#ifndef RANGEOBJECT_H
#define RANGEOBJECT_H

#include <stdlib.h>
#include <stdint.h>

typedef struct _range {
    int32_t *ref;

    int32_t start;
    int32_t stop;
    int32_t step;
} *RangeObject;

static inline RangeObject newRangeObject(struct _range defaults) {
    RangeObject ro = (RangeObject)calloc(1, sizeof(*ro));
    if (!ro) return NULL;

    ro->start = defaults.start;
    ro->stop = defaults.stop;
    ro->step = (defaults.step) ? defaults.step : 1;
    ro->ref = defaults.ref;

    return ro;
}

static inline void deleteRangeObject(RangeObject *ro) {
    if (!ro) return;

    free(*ro);
    *ro = NULL;
}

#define range(...) newRangeObject((struct _range){__VA_ARGS__})

#define forrange_primitive(r, it, ...) \
    for (RangeObject r = range(__VA_ARGS__); r; deleteRangeObject(&r)) \
        for (int32_t it = (r->ref ? (*r->ref = r->start) : r->start); \
             it < r->stop; \
             it += r->step, (void)(r->ref && (*r->ref = it)))

#define forrange(...) forrange_primitive(concat_layer2(_r, __COUNTER__), concat_layer2(_it, __COUNTER__), __VA_ARGS__)

#endif

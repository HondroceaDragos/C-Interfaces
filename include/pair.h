#ifndef PAIR_H
#define PAIR_H

#include "./helpers.h"

#define PairType(fst, snd) \
    typedef struct concat_layer2(_pair_, concat_layer2(fst, snd)) { \
        const fst first; \
        const snd second; \
    } concat_layer2(Pair_, concat_layer2(fst, snd)); \
    \
    static inline concat_layer2(Pair_, concat_layer2(fst, snd)) concat_layer2(newPair_, concat_layer2(fst, snd))( \
        struct concat_layer2(_pair_, concat_layer2(fst, snd)) defaults) { \
        return (concat_layer2(Pair_, concat_layer2(fst, snd))) { \
            .first = defaults.first, \
            .second = defaults.second \
        }; \
    } \
    \
    deleteType(Pair(fst, snd)) \

#define Pair(fst, snd) concat_layer2(Pair_, concat_layer2(fst, snd))
#define newPair(fst, snd, ...) concat_layer2(newPair_, concat_layer2(fst, snd))((struct concat_layer2(_pair_, concat_layer2(fst, snd)))__VA_ARGS__)

#endif

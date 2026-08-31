#ifndef OPTION_H
#define OPTION_H

#include "./helpers.h"

typedef enum : ui8 {
    Left = 1,
    Right = 2
} OptionTag;

#define OptionType(type) \
    typedef struct concat_layer2(_option_, type) { \
        OptionTag side; \
        union { \
            const ui8 *left; \
            type right; \
        }; \
    } concat_layer2(Option_, type); \
    \
    typedef struct _option_##type##_default { \
        OptionTag side; \
        const ui8 *left; \
        type right; \
    } concat_layer2(Od_, type); \
    \
    static inline concat_layer2(Option_, type) concat_layer2(newOption_, type)(concat_layer2(Od_, type) defaults) { \
        concat_layer2(Option_, type) e = {}; \
        \
        if (defaults.left && defaults.right) raise(ERROR, "Cannot create a " RED "dual-type option" RESET "."); \
        if (!defaults.side) raise(ERROR, "Cannot create a " RED "dual-type option" RESET "."); \
        \
        e.side = defaults.side; \
        if (e.side == Left) e.left = defaults.left; \
        if (e.side == Right) e.right = defaults.right; \
        \
        return e; \
    } \
    deleteType(Option(type))

#define Option(type) concat_layer2(Option_, type)
#define newOption(type, ...) concat_layer2(newOption_, type)((concat_layer2(Od_, type)){__VA_ARGS__})

#define Some(x) (x).right
#define None(x) (x).left

#endif
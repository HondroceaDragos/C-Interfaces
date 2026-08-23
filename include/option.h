#ifndef OPTION_H
#define OPTION_H

#include "./helpers.h"

typedef enum : ui8 {
    Left,
    Right
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
        const ui8 *left; \
        type right; \
    } concat_layer2(Od_, type); \
    \
    static inline concat_layer2(Option_, type) concat_layer2(newOption_, type)(concat_layer2(Od_, type) defaults) { \
        concat_layer2(Option_, type) e = {}; \
        \
        if (defaults.left && defaults.right) { \
            raise(ERROR, "Cannot create a " RED "dual-type option" RESET "."); \
        } \
        \
        if (defaults.left) { \
            e.side = Left; \
            e.left = defaults.left; \
        } else if (defaults.right) { \
            e.side = Right; \
            e.right = defaults.right; \
        } else { \
            e.side = Left; \
            e.left = "Unspecified Error"; \
        } \
        \
        return e; \
    }

#define Option(type) concat_layer2(Option_, type)
#define newOption(type, ...) concat_layer2(newOption_, type)((concat_layer2(Od_, type)){__VA_ARGS__})

#define Some(x) (x).right
#define None(x) (x).left

#endif
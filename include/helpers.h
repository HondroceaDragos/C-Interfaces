#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "./fortype.h"

#define concat_layer1(a, b) a##b
#define concat_layer2(a, b) concat_layer1(a, b)

#define defer(func) [[gnu::cleanup(func)]]

#define deleteDefine(type) void concat_layer2(_delete_, type)(type *self)
#define deleteType(type) void concat_layer2(_delete_, type)(type *obj) {}
#define delete(type) concat_layer2(_delete_, type)

#endif

#ifndef HELPERS_H
#define HELPERS_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "./fortype.h"

#define defer(func) [[gnu::cleanup(func)]]
#define deleteDefine(type) void concat_layer1(delete_, type)(type obj) {}
#define delete(type) concat_layer1(delete_, type)

#define concat_layer1(a, b) a##b
#define concat_layer2(a, b) concat_layer1(a, b)

#endif

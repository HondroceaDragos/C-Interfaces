#ifndef RAISE_H
#define RAISE_H

#include <stdio.h>
#include <stdlib.h>

#define RED "\033[38;2;234;23;12m"
#define YELLOW "\033[38;2;255;234;0m"
#define RESET "\033[0m"

#undef ERROR

typedef enum _raise_code : ui8 {
    ERROR,
    WARNING
} RaiseCode;

#define raiseError(fmt, ...) \
    do { \
        fprintf(stderr, "[" RED "ERROR" RESET "] " RED "%s:%d" RESET " " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
        exit(EXIT_FAILURE); \
    } while (false)

#define raiseWarning(fmt, ...) \
    do { \
        fprintf(stderr, "[" YELLOW "WARNING" RESET "] " YELLOW "%s:%d" RESET " " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (false)

#define raise(code, ...) \
    do { \
        if ((code) == WARNING) raiseWarning(__VA_ARGS__); \
        else raiseError(__VA_ARGS__); \
    } while(false)

#endif

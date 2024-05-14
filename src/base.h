#ifndef __BASE_H__

#define __BASE_H__

#include <stdio.h>
#include <stdlib.h>

#define _AXE_CHECK(cond, ...)                                                  \
    do {                                                                       \
        if (!(cond)) {                                                         \
            fprintf(stderr, "AXE_CHECK failed at %s:%d\n", __FILE__,           \
                    __LINE__);                                                 \
            fprintf(stderr, __VA_ARGS__);                                      \
            fprintf(stderr, "\n");                                             \
            fflush(stderr);                                                    \
        }                                                                      \
    } while (0)

#ifdef DNDEBUG
#define AXE_CHECK ((void)0)
#else
#define AXE_CHECK(cond, ...) _AXE_CHECK(cond, __VA_ARGS__)
#endif

#endif // __BASE_H__

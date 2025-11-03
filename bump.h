#define BUMPC_DIM 2
#define BUMPC_FLOAT float
// margin of error
#define BUMPC_DELTA 1e-5 

#define BUMPC_USELIBC // You must -lm if using libc
                      // includes: math, stdlib, float
                      //
                      // If this is not defined, you must define functions equivilant to the following std ones:
                      // #define BUMPC_SQRT sqrt
                      // #define BUMPC_ABS fabs
                      // #define BUMPC_MIN fminf
                      // #define BUMPC_FLOATMAX FLT_MAX
                      // #define BUMPC_SORT qsort
                      //
                      // Additionally, after calling bumpc_init(), the ctx.alloc_func, etc must be set

#include "bump_generic.c"

# port of bump.lua to C
There may be bugs.

This is generic for demensions.

To use, copy bump_generic.c into your project and create two files:

## bump.h
```
#define BUMPC_USELIBC
#define BUMPC_DIM 2
#define BUMPC_FLOAT float
#include "bump_generic.c"
```

## bump.c
```
#define BUMPC_IMPLEMENTATION
#include "bump.h"
```


see `raylib_demo.c` for a demo using raylib

build it with `gcc raylib_demo.c bump.c -lraylib -lm -o demo`

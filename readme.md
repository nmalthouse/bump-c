# port of bump.lua to C

wip

Should work with a any number of dimensions.

This is generic for demensions.

To use copy bump_generic.c into your project and create two files

## bump.h
```
#define BUMPC_DIM 2
#define BUMPC_FLOAT float
#include "bump_generic.c"
```

## bump.c
```
#define BUMPC_IMPLEMENTATION
#include "bump.h"
```

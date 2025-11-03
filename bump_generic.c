#include <stddef.h>

#ifndef BUMPC_FLOAT
    #define BUMPC_FLOAT float
#endif

#ifndef BUMPC_DIM
    #define BUMPC_DIM 2
#endif

#ifndef BUMPC_BOOL
    #define BUMPC_BOOL char
    #define BUMPC_TRUE 1
    #define BUMPC_FALSE 0
#endif

#ifndef BUMPC_DELTA
    #define BUMPC_DELTA  1e-5
#endif

typedef struct bumpc_Vec {
    BUMPC_FLOAT data[BUMPC_DIM];
} bumpc_Vec; 

#if BUMPC_DIM==2
    bumpc_Vec bumpc_VecNew(BUMPC_FLOAT a, BUMPC_FLOAT b);
#endif

typedef struct bumpc_Aabb {
    bumpc_Vec pos;
    bumpc_Vec ext;
} bumpc_Aabb;

#define BUMPC_GOOD 1
#define BUMPC_BAD 0 
typedef struct bumpc_LbRes {
    BUMPC_FLOAT ti1;
    BUMPC_FLOAT ti2;
    bumpc_Vec norm1;
    bumpc_Vec norm2;
    char status;
} bumpc_LbRes;

typedef struct bumpc_CollisionResult {
    BUMPC_BOOL overlaps;
    BUMPC_FLOAT ti; 
    bumpc_Vec delta;  // The delta applied to moved if no collision occured
    bumpc_Vec norm;   // normal of collision
    bumpc_Vec touch;  // new value for moved.pos if collision
    bumpc_Aabb moved;
    bumpc_Aabb other;
    size_t id;        // index of `moved` in the passed input list
    char status;      // == BUMPC_GOOD when a collision occured
} bumpc_CollisionResult;

typedef void *(*bumpc_alloc_func)(void *opaque, size_t items, size_t size);
typedef void (*bumpc_free_func)(void *opaque, void *address);
typedef void *(*bumpc_realloc_func)(void *opaque, void *address, size_t items, size_t size);

typedef struct bumpc_Ctx {
    void* useralloc;
    bumpc_alloc_func alloc_func;
    bumpc_free_func free_func;
    bumpc_realloc_func realloc_func;

    bumpc_CollisionResult * output;
    size_t output_count;
} bumpc_Ctx;

bumpc_Ctx bumpc_init();
void bumpc_deinit(bumpc_Ctx*);

#ifdef BUMPC_USELIBC
    #include <math.h>
    #include <stdlib.h>
    #include <float.h>
    #define BUMPC_SQRT sqrt
    #define BUMPC_ABS fabs
    #define BUMPC_MIN fminf
    #define BUMPC_SORT qsort

    #ifndef BUMPC_FLOATMAX
        #define BUMPC_FLOATMAX FLT_MAX
    #endif
#endif
        
// Determine collisions when moving `moved` to goal against list of aabb's in `to_check`
// ctx.output holds a sorted list of results
// Returns the number of results stored in `ctx.output`
size_t bumpc_detectCollisionList(
                          bumpc_Ctx*,
                          bumpc_Aabb moved, 
                          bumpc_Aabb * to_check, 
                          size_t to_check_count,
                          bumpc_Vec goal);

BUMPC_FLOAT bumpc_vec_len(bumpc_Vec);
BUMPC_BOOL bumpc_floatEql(BUMPC_FLOAT , BUMPC_FLOAT );
#ifdef BUMPC_IMPLEMENTATION


#ifdef BUMPC_USELIBC
void *bumpc_def_alloc_func(void *opaque, size_t items, size_t size) {
    return malloc(items * size);
}
void bumpc_def_free_func(void *opaque, void *address) {
    free(address);
}
void *bumpc_def_realloc_func(void *opaque, void *address, size_t items, size_t size) {
    return realloc(address, items * size);
}
#endif  //BUMPC_USELIBC
        
bumpc_Ctx bumpc_init(){
    return (bumpc_Ctx){
        #ifdef BUMPC_USELIBC
        .alloc_func = bumpc_def_alloc_func,
        .realloc_func = bumpc_def_realloc_func,
        .free_func = bumpc_def_free_func,
        .useralloc = NULL,
        #else
        .alloc_func = NULL,
        .realloc_func = NULL,
        .free_func = NULL,
        .useralloc = NULL,
        #endif 
        .output = NULL,
        .output_count = 0,
    };
}

void bumpc_deinit(bumpc_Ctx * ctx){
    if(ctx->free_func != NULL){
        ctx->free_func(ctx->useralloc, ctx->output);
        ctx->output = NULL;
    }
}

#if BUMPC_DIM==2
    bumpc_Vec bumpc_VecNew(BUMPC_FLOAT a, BUMPC_FLOAT b){
        bumpc_Vec new;
        new.data[0] = a;
        new.data[1] = b;
        return new;
    }
#endif

BUMPC_BOOL bumpc_floatEql(BUMPC_FLOAT a, BUMPC_FLOAT b) {
    if(BUMPC_ABS(a - b) < BUMPC_DELTA )
        return BUMPC_TRUE;
    return BUMPC_FALSE;
}

bumpc_Vec bumpc_vec_sub(bumpc_Vec a, bumpc_Vec b){
    bumpc_Vec ret = a;
    for(int i = 0; i < BUMPC_DIM; i++){ ret.data[i] -= b.data[i]; }
    return ret;
}

bumpc_Vec bumpc_vec_add(bumpc_Vec a, bumpc_Vec b){
    bumpc_Vec ret = a;
    for(int i = 0; i < BUMPC_DIM; i++){ ret.data[i] += b.data[i]; }
    return ret;
}

BUMPC_FLOAT bumpc_vec_len(bumpc_Vec a){
    BUMPC_FLOAT sum = 0;
    for(int i = 0; i < BUMPC_DIM; i++){ sum += a.data[i]; }
    return BUMPC_SQRT(sum);
}

BUMPC_FLOAT bumpc_nearest(BUMPC_FLOAT test_point, BUMPC_FLOAT a, BUMPC_FLOAT b){
    return  (abs(a - test_point) < abs(b - test_point)) ? a : b;
}

BUMPC_FLOAT bumpc_sign(BUMPC_FLOAT a) {
    if(a < 0.0)
        return -1;
    if(bumpc_floatEql(0.0, a))
        return 0;
    return 1;
}

bumpc_Vec bumpc_nearestCorner(bumpc_Aabb bb, bumpc_Vec point){
    bumpc_Vec ret;
    for(int i = 0; i < BUMPC_DIM; i++){
        ret.data[i] = bumpc_nearest(point.data[i], bb.pos.data[i], bb.pos.data[i] + bb.ext.data[i]);
    }
    return ret;
}

BUMPC_BOOL bumpc_containsPoint(bumpc_Aabb bb, bumpc_Vec point){
    BUMPC_BOOL ret = BUMPC_TRUE;
    for(int i = 0; i< BUMPC_DIM; i++){
        ret = ret && 
            point.data[i] - bb.pos.data[i] > BUMPC_DELTA && 
            bb.pos.data[i] + bb.ext.data[i] - point.data[i] > BUMPC_DELTA;
    }
    return ret;
}

bumpc_Aabb bumpc_minkowsky(bumpc_Aabb a, bumpc_Aabb b) {
    bumpc_Aabb ret;
    for(int i = 0; i < BUMPC_DIM; i++){
        ret.pos.data[i] = b.pos.data[i] - a.pos.data[i] - a.ext.data[i];
        ret.ext.data[i] = a.ext.data[i] + b.ext.data[i];
    }
    return ret;
}

BUMPC_BOOL bumpc_lbClip(bumpc_Vec norm, 
                  BUMPC_FLOAT p, 
                  BUMPC_FLOAT q, 
                  BUMPC_FLOAT * ti1, 
                  BUMPC_FLOAT * ti2,
                  bumpc_Vec * n1,
                  bumpc_Vec * n2){
    if(bumpc_floatEql(p, 0.0)){
        if(q <= 0) return BUMPC_FALSE;
    }
    else {
        const BUMPC_FLOAT r  = q / p;
        if(p < 0){
            if(r > *ti2) return BUMPC_FALSE;

            if(r > *ti1){
                *ti1 = r;
                *n1 = norm;
            }
        }
        else{
            if(r < *ti1) return BUMPC_FALSE;
            if(r < *ti2){
                *ti2 = r;
                *n2 = norm;

            }
        }
    }
    return BUMPC_TRUE;
}


BUMPC_BOOL bumpc_liangBarskyLineClip(bumpc_Aabb clip_window, 
                                      bumpc_Vec start, 
                                      bumpc_Vec end,
                                      bumpc_LbRes * res
                                      ){
    bumpc_Vec delta = bumpc_vec_sub(end, start);

    bumpc_Vec norm1 = {0};
    bumpc_Vec norm2 = {0};

    for(int i = 0; i < BUMPC_DIM; i++){
        bumpc_Vec nneg = {0};
        bumpc_Vec npos = {0};
        nneg.data[i] = -1;
        npos.data[i] = 1;
        if (bumpc_lbClip(
        nneg, -delta.data[i], start.data[i] - clip_window.pos.data[i], &res->ti1, &res->ti2, &norm1, &norm2) 
            == BUMPC_FALSE)
            return BUMPC_FALSE;
        if (bumpc_lbClip(npos, delta.data[i], clip_window.pos.data[i] + clip_window.ext.data[i] - start.data[i], &res->ti1, &res->ti2, &norm1, &norm2) == BUMPC_FALSE)
            return  BUMPC_FALSE;
    }
    res->norm1 = norm1;
    res->norm2 = norm2;
    return BUMPC_TRUE;
}

bumpc_CollisionResult bumpc_detectCollisionAabb(bumpc_Aabb moved, bumpc_Aabb other, bumpc_Vec goal){
    const bumpc_Vec delta = bumpc_vec_sub(goal, moved.pos);
    const bumpc_Aabb mdiff = bumpc_minkowsky(moved, other);
    BUMPC_BOOL overlaps = BUMPC_FALSE;

    BUMPC_FLOAT ti = 0;
    bumpc_Vec norm = {0};

    // If the Minkowsky diff intersects the origin it is already overlapping
    if(bumpc_containsPoint(mdiff, (bumpc_Vec){0})){
        const bumpc_Vec point = bumpc_nearestCorner(mdiff, (bumpc_Vec){0});
        BUMPC_FLOAT area = -1;
        for(int i = 0; i < BUMPC_DIM; i++){
            area *= BUMPC_MIN(moved.ext.data[i], BUMPC_ABS(point.data[i]));
        }
        //ti = -wi * hi; // ti is the negative area of intersection
        ti = area;
        overlaps = BUMPC_TRUE;
    }
    else {
        bumpc_LbRes lb; 
        lb.ti1 = -BUMPC_FLOATMAX;
        lb.ti2 = BUMPC_FLOATMAX;
        BUMPC_BOOL status = bumpc_liangBarskyLineClip(mdiff, (bumpc_Vec){0},delta, &lb);
        if(status 
                && lb.ti1 < 1.0 
                && (BUMPC_ABS(lb.ti1 - lb.ti2) >= BUMPC_DELTA) 
                && (0 < lb.ti1 + BUMPC_DELTA  || bumpc_floatEql(lb.ti1, 0) && lb.ti2 > 0)){
            ti = lb.ti1;
            norm = lb.norm1;
            overlaps = BUMPC_FALSE;
        }
        else {
            return (bumpc_CollisionResult){.status = BUMPC_BAD};
        }
    }

    bumpc_Vec tv = {0};
    if(overlaps){
        // If we aren't moving, teleport to nearest edge
        if(bumpc_floatEql(bumpc_vec_len(delta),  0.0)){
            bumpc_Vec np = bumpc_nearestCorner(mdiff, (bumpc_Vec){0});
            size_t mi = 100;
            BUMPC_FLOAT min =  BUMPC_FLOATMAX;
            //Find the minimum component and set all others to zero
            {
                for(int i = 0; i < BUMPC_DIM; i++){
                    if(BUMPC_ABS(np.data[i]) < min){
                        min = BUMPC_ABS(np.data[i]);
                        mi = i;
                    }
                }
                for(int i = 0; i < BUMPC_DIM; i++){
                    if(i != mi) 
                        np.data[i] = 0;
                }
                for(int i = 0; i < BUMPC_DIM; i++){
                    norm.data[i] = bumpc_sign(np.data[i]);
                    tv.data[i] = moved.pos.data[i] + np.data[i];
                }
            }
        } 
        else {
            bumpc_LbRes lb;
            lb.ti1 = -BUMPC_FLOATMAX;
            lb.ti2 = 1;

            BUMPC_BOOL status =  bumpc_liangBarskyLineClip(mdiff, (bumpc_Vec){0},delta, &lb);
            if(status != BUMPC_GOOD) return (bumpc_CollisionResult){.status = BUMPC_BAD};
            norm = lb.norm1;
            ti = lb.ti1;

            for(int i = 0; i < BUMPC_DIM; i++){
                tv.data[i] = moved.pos.data[i] + delta.data[i] * ti;
            }
        }
    }
    else {
        for(int i = 0; i < BUMPC_DIM; i++){
            tv.data[i] = moved.pos.data[i] + delta.data[i] * ti;
        }
    }
    return (bumpc_CollisionResult){
        .overlaps = overlaps,
        .ti = ti,
        .delta = delta,
        .norm = norm,
        .touch = tv,
        .status = BUMPC_GOOD,
        .moved = moved,
        .other = other,
    };
}

BUMPC_FLOAT bumpc_squareDist(bumpc_Aabb bb1, bumpc_Aabb bb2){
    BUMPC_FLOAT sum = 0;
    for(int i = 0; i < BUMPC_DIM; i++){
        BUMPC_FLOAT a = bb1.pos.data[i] - bb2.pos.data[i] + (bb1.ext.data[i] - bb2.ext.data[i]) / 2;
        sum += a * a;
    }
    return sum;
}

int bumpc_compareCollisionResult(const void* a, const void* b){
    const bumpc_CollisionResult * lhs = a;
    const bumpc_CollisionResult * rhs = b;

    if(bumpc_floatEql(lhs->ti, rhs->ti)){
        return bumpc_squareDist(lhs->moved, lhs->other) >  bumpc_squareDist(rhs->moved, rhs->other);
    }
    return lhs->ti > rhs->ti;
}

size_t bumpc_detectCollisionList(bumpc_Ctx * ctx,
                                 bumpc_Aabb moved, 
                                 bumpc_Aabb * to_check, 
                                 size_t to_check_count,
                                 bumpc_Vec goal){
    size_t grow_amt = 10;

    size_t col_count = 0;
    for(int i = 0; i < to_check_count; i++){
        const bumpc_Aabb other = to_check[i];
        bumpc_CollisionResult col = bumpc_detectCollisionAabb(moved, other, goal);
        if(col.status == BUMPC_GOOD){
            if(col_count >= ctx->output_count){
                ctx->output_count += grow_amt;
                ctx->output = ctx->realloc_func(ctx->useralloc, ctx->output,ctx->output_count,  sizeof(bumpc_CollisionResult));
            }

            col.id = i;
            ctx->output[col_count] = col;
            col_count += 1;
        }
    }

    if(col_count > 0){
        BUMPC_SORT((void*)ctx->output, col_count, sizeof(bumpc_CollisionResult), bumpc_compareCollisionResult);
    }

    return col_count;
}


#endif //BUMPC_IMPLEMENTATION

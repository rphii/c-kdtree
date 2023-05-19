/* MIT License

Copyright (c) 2023 rphii

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#ifndef KDTREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> /* INFINITY */

#include "vec.h"

#define KDT_MAX_DIM     3

typedef struct KDTreeNode {
    ssize_t left;
    ssize_t right;
    size_t index;
} KDTreeNode;

#define KDTREE_SWAP(T, x,y)   {T t = x; x = y; y = t; }

typedef struct Point3d {
    double x[3];
} Point3d;

VEC_INCLUDE(Vec3d, vec3d, Point3d, BY_VAL);

VEC_INCLUDE(Vec1d, vec1d, double, BY_VAL);
VEC_INCLUDE(KDTreeBuckets, kdtree_buckets, KDTreeNode, BY_REF);

typedef struct KDTree {
    Vec3d *arr;
    KDTreeBuckets buckets;
    size_t dim;
    ssize_t root;
} KDTree;

void buckets_print(KDTree *kdt);
void vec3d_print(Vec3d *vec);


int kdtree_create(KDTree *kdt, Vec3d *arr);
ssize_t kdtree_nearest(KDTree *kdt, Point3d *pt, double *dist);


/*
 * N = name of the kdtree struct
 * A = abbreviation of the kdtree functions
 * T = name of the type struct
 * VN = name of the vector type based around T
 * VA = abbreviation of the vector based around T (assumes that implementations are present)
 */

#define KDTREE_INCLUDE(N, A, T, VN) \
    typedef struct N { \
        KDTreeBuckets buckets; \
        VN *ref; \
        size_t dim; \
        ssize_t root; \
    } N; \
    \
    int A##_create(N *kdt, VN *ref, size_t dim); \
    ssize_t A##_nearest(N *kdt, VN *pt, double *squared_dist); \
    void A##_free(N *kdt); \


#define KDTREE_IMPLEMENT(N, A, T, VN, VA) \
    KDTREE_IMPLEMENT_STATIC_MEDIAN(N, A, T, VN, VA); \
    KDTREE_IMPLEMENT_STATIC_CREATE(N, A, T, VN, VA); \
    KDTREE_IMPLEMENT_CREATE(N, A, T, VN, VA); \
    KDTREE_IMPLEMENT_STATIC_DIST(N, A, T, VN, VA); \
    KDTREE_IMPLEMENT_STATIC_NEAREST(N, A, T, VN, VA); \
    KDTREE_IMPLEMENT_NEAREST(N, A, T, VN, VA); \
    KDTREE_IMPLEMENT_FREE(N, A, T, VN, VA); \

#define KDTREE_IMPLEMENT_STATIC_MEDIAN(N, A, T, VN, VA) \
    static inline ssize_t A##_static_median(N *kdt, size_t i0, size_t iE, size_t i_dim) \
    { \
        assert(kdt); \
        assert(kdt->ref); \
        if(iE <= i0) return -1LL; \
        if(iE == i0 + 1) return i0; \
        size_t md = i0 + (iE - i0) / 2; \
        size_t store = 0; \
        size_t p = 0; \
        for(;;) { \
            size_t i_pivot = kdtree_buckets_get_at(&kdt->buckets, md)->index; \
            T pivot = VA##_get_at(kdt->ref, i_pivot + i_dim); \
            KDTREE_SWAP(T, kdt->buckets.items[md]->index, kdt->buckets.items[iE - 1]->index); \
            store = i0; \
            for(p = i0; p < iE; p++) { \
                size_t i_p = kdtree_buckets_get_at(&kdt->buckets, p)->index; \
                T p_x = VA##_get_at(kdt->ref, i_p + i_dim); \
                if(p_x < pivot) { \
                    if(p != store) { \
                        KDTREE_SWAP(T, kdt->buckets.items[p]->index, kdt->buckets.items[store]->index); \
                    } \
                    store++; \
                } \
            } \
            KDTREE_SWAP(T, kdt->buckets.items[store]->index, kdt->buckets.items[iE - 1]->index); \
            /* median has duplicate values */ \
            size_t i_s = kdtree_buckets_get_at(&kdt->buckets, store)->index; \
            T v_s = VA##_get_at(kdt->ref, i_s + i_dim); \
            size_t i_m = kdtree_buckets_get_at(&kdt->buckets, md)->index; \
            T v_m = VA##_get_at(kdt->ref, i_m + i_dim); \
            if(v_s == v_m) { return md; } \
            if(store > md) iE = store; \
            else i0 = store; \
        } \
        return 0; \
    }

#define KDTREE_IMPLEMENT_STATIC_CREATE(N, A, T, VN, VA) \
    static inline ssize_t A##_static_create(N *kdt, size_t i0, size_t iE, size_t i_dim) \
    { \
        assert(kdt->ref); \
        if(!iE) return -1LL; \
        ssize_t m = A##_static_median(kdt, i0, iE, i_dim); \
        if(m >= 0) { \
            i_dim = (i_dim + 1) % kdt->dim; \
            KDTreeNode *n = kdtree_buckets_get_at(&kdt->buckets, m); \
            n->left = A##_static_create(kdt, i0, m, i_dim); \
            n->right = A##_static_create(kdt, m + 1, iE, i_dim); \
        } \
        return m; \
    }

#define KDTREE_IMPLEMENT_CREATE(N, A, T, VN, VA) \
    int A##_create(N *kdt, VN *ref, size_t dim) \
    { \
        assert(kdt); \
        assert(ref); \
        assert(dim); \
        kdt->ref = ref; \
        kdt->dim = dim; \
        assert(!(ref->len % dim)); \
        for(size_t i = 0; i < ref->len; i += dim) { \
            kdtree_buckets_push_back(&kdt->buckets, &(KDTreeNode){.index = i}); \
        } \
        kdt->root = A##_static_create(kdt, 0, kdt->buckets.len, 0); \
        return 0; \
    }


#define KDTREE_IMPLEMENT_STATIC_DIST(N, A, T, VN, VA) \
    static inline double A##_static_dist(N *kdt, ssize_t index, VN *pt) \
    { \
        size_t i_dim = kdt->dim; \
        T d = 0; \
        while(i_dim--) { \
            T a = VA##_get_at(kdt->ref, index * kdt->dim + i_dim); \
            T b = VA##_get_at(pt, i_dim); \
            double t = a - b; \
            d += t * t; \
        } \
        return d; \
    }

#define KDTREE_IMPLEMENT_STATIC_NEAREST(N, A, T, VN, VA) \
    static inline void A##_static_nearest(N *kdt, ssize_t root, VN *pt, size_t i_dim, ssize_t *best, double *dist) \
    { \
        assert(kdt); \
        assert(best); \
        assert(dist); \
        assert(pt); \
        if(root < 0) return; \
        double d = A##_static_dist(kdt, root, pt); \
        /*printf("dist [%zu] %.2f, i_dim %zu\n", root*kdt->dim, d, i_dim);*/\
        T a = VA##_get_at(kdt->ref, root + i_dim); \
        T b = VA##_get_at(pt, i_dim); \
        double dx = a - b; \
        double dx2 = dx * dx; \
        if(!*best || d < *dist) { \
            *dist = d; \
            *best = root; \
        } \
        /* if chance of exact match is high */ \
        if(!*dist) return; \
        if(++i_dim >= kdt->dim) i_dim = 0; \
        KDTreeNode *r = kdtree_buckets_get_at(&kdt->buckets, root); \
        if(dx2 <= *dist) A##_static_nearest(kdt, dx > 0 ? r->left : r->right, pt, i_dim, best, dist); \
        A##_static_nearest(kdt, dx > 0 ? r->right : r->left, pt, i_dim, best, dist); \
    }

#define KDTREE_IMPLEMENT_NEAREST(N, A, T, VN, VA) \
    ssize_t A##_nearest(N *kdt, VN *pt, double *squared_dist) \
    { \
        assert(kdt); \
        assert(pt); \
        double dist_temp = 0; \
        if(!squared_dist) squared_dist = &dist_temp; \
        *squared_dist = INFINITY; \
        ssize_t best = -1LL; \
        A##_static_nearest(kdt, kdt->root, pt, 0, &best, squared_dist); \
        return best * kdt->dim; \
    }

#define KDTREE_IMPLEMENT_FREE(N, A, T, VN, VA) \
    void A##_free(N *kdt) \
    { \
        assert(kdt); \
        kdtree_buckets_free(&kdt->buckets); \
        memset(kdt, 0, sizeof(*kdt)); \
    }

KDTREE_INCLUDE(KDTrD, kdtrd, double, Vec1d);

#if 0

typedef struct KDTreeNode {
    struct KDTreeNode *left;
    struct KDTreeNode *right;
    size_t index;
} KDTreeNode;

VEC_INCLUDE(KDTreeBuckets, kdtree_buckets, KDTreeNode, BY_VAL);
typedef struct KDTree {
    KDTreeBuckets buckets;
    KDTreeNode *root;
    size_t dim;
} KDTree;


/* public functions */
void kdtree_create(KDTree *kdt, Vec3d); /* calls */
size_t kdtree_nearest(KDTree *kdt, KDTreeValue find, double *best_dist);
#endif


#define KDTREE_H
#endif


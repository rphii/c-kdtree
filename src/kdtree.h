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

#define KDTREE_SWAP(T, x,y)   {T t = x; x = y; y = t; }
#define KDTREE_DEBUG    1

typedef struct KDTreeNode {
    ssize_t left;
    ssize_t right;
    size_t index;
} KDTreeNode;

VEC_INCLUDE(KDTreeBuckets, kdtree_buckets, KDTreeNode, BY_REF);

/*
 * N = name of the kdtree struct
 * A = abbreviation of the kdtree functions
 * T = name of the type struct
 */

#define KDTREE_INCLUDE(N, A, T) \
    typedef struct N { \
        KDTreeBuckets buckets; \
        T* ref; \
        size_t len;   /* length of ref array */ \
        size_t dim;   /* count of dimensions */ \
        size_t stride; \
        ssize_t root; /* root returned from create */ \
    } N; \
    \
    int A##_create(N *tree , T *ref, size_t len, size_t dim, size_t offset, size_t stride); \
    ssize_t A##_nearest(N *tree , T *pt, double *squared_dist); \
    void A##_free(N *tree ); \


#define KDTREE_IMPLEMENT(N, A, T) \
    KDTREE_IMPLEMENT_STATIC_GET_AT(N, A, T); \
    KDTREE_IMPLEMENT_STATIC_MEDIAN(N, A, T); \
    KDTREE_IMPLEMENT_STATIC_CREATE(N, A, T); \
    KDTREE_IMPLEMENT_CREATE(N, A, T); \
    KDTREE_IMPLEMENT_STATIC_DISTANCE(N, A, T); \
    KDTREE_IMPLEMENT_STATIC_NEAREST(N, A, T); \
    KDTREE_IMPLEMENT_NEAREST(N, A, T); \
    KDTREE_IMPLEMENT_FREE(N, A, T); \

#define KDTREE_IMPLEMENT_STATIC_GET_AT(N, A, T) \
    T A##_static_get_at(T *ref, size_t index, size_t len) { \
        if(KDTREE_DEBUG) { \
            if(index >= len) { \
                printf("\n>> accessing index %zu / len %zu\n", index, len); \
            } \
        } \
        assert(index < len); \
        return ref[index]; \
    }

#define KDTREE_IMPLEMENT_STATIC_MEDIAN(N, A, T) \
    static inline ssize_t A##_static_median(N *tree , size_t i0, size_t iE, size_t i_dim) { \
        assert(tree); \
        assert(tree->ref); \
        if(iE <= i0) return -1LL; \
        if(iE == i0 + 1) return i0; \
        size_t md = i0 + (iE - i0) / 2; \
        size_t store = 0; \
        size_t p = 0; \
        for(;;) { \
            size_t i_pivot = kdtree_buckets_get_at(&tree->buckets, md)->index; \
            T pivot = A##_static_get_at(tree->ref, i_pivot + i_dim, tree->len); \
            KDTREE_SWAP(T, tree->buckets.items[md]->index, tree->buckets.items[iE - 1]->index); \
            store = i0; \
            for(p = i0; p < iE - 1; p++) { \
                size_t i_p = kdtree_buckets_get_at(&tree->buckets, p)->index; \
                T p_x = A##_static_get_at(tree->ref, i_p + i_dim, tree->len); \
                if(p_x < pivot) { \
                    if(p != store) { \
                        KDTREE_SWAP(T, tree->buckets.items[p]->index, tree->buckets.items[store]->index); \
                    } \
                    store++; \
                    assert(store < tree->buckets.len); \
                } \
            } \
            KDTREE_SWAP(T, tree->buckets.items[store]->index, tree->buckets.items[iE - 1]->index); \
            /* median has duplicate values */ \
            size_t i_s = kdtree_buckets_get_at(&tree->buckets, store)->index; \
            T v_s = A##_static_get_at(tree->ref, i_s + i_dim, tree->len); \
            size_t i_m = kdtree_buckets_get_at(&tree->buckets, md)->index; \
            T v_m = A##_static_get_at(tree->ref, i_m + i_dim, tree->len); \
            if(v_s == v_m) { return md; } \
            if(store > md) iE = store; \
            else i0 = store; \
        } \
        return 0; \
    }

#define KDTREE_IMPLEMENT_STATIC_CREATE(N, A, T) \
    static inline ssize_t A##_static_create(N *tree , size_t i0, size_t iE, size_t i_dim) { \
        assert(tree->ref); \
        if(!iE) return -1LL; \
        ssize_t m = A##_static_median(tree, i0, iE, i_dim); \
        if(m >= 0) { \
            i_dim = (i_dim + 1) % tree->dim; \
            KDTreeNode *n = kdtree_buckets_get_at(&tree->buckets, m); \
            n->left = A##_static_create(tree, i0, m, i_dim); \
            n->right = A##_static_create(tree, m + 1, iE, i_dim); \
        } \
        return m; \
    }

#define KDTREE_IMPLEMENT_CREATE(N, A, T) \
    int A##_create(N *tree , T *ref, size_t len, size_t dim, size_t offset, size_t stride) { \
        assert(dim); \
        assert(tree); \
        assert(ref); \
        tree->ref = ref; \
        tree->len = len; \
        tree->dim = dim; \
        if(!stride) stride = dim; \
        tree->stride = stride; \
        for(size_t i = offset; i < len; i += stride) { \
            for(size_t j = 0; j < stride; j += dim) { \
                kdtree_buckets_push_back(&tree->buckets, &(KDTreeNode){.index = i + j}); \
            } \
        } \
        tree->len = tree->buckets.len * tree->dim; \
        tree->root = A##_static_create(tree, 0, tree->buckets.len, 0); \
        return 0; \
    }

#define KDTREE_IMPLEMENT_STATIC_DISTANCE(N, A, T) \
    double A##_static_distance(size_t dim, T *x, T *y) { \
        double d = 0; \
        for(size_t i = 0; i < dim; i++) { \
            d += (x[i] - y[i]) * (x[i] - y[i]); \
        } \
        return d; \
    }

#define KDTREE_IMPLEMENT_STATIC_NEAREST(N, A, T) \
    static inline void A##_static_nearest(N* tree, ssize_t root, T* pt, size_t i_dim, ssize_t *best, double *best_dist) { \
        if(root < 0) return; \
        /* Get the current node from the KDTree */ \
        KDTreeNode* node = kdtree_buckets_get_at(&tree->buckets, root); \
        /* Calculate the distance from the target point to the current node */ \
        double currentDistance = A##_static_distance(tree->dim, pt, &(tree->ref[node->index])); \
        if(*best < 0 || currentDistance < *best_dist) { \
            *best = root; \
            *best_dist = currentDistance; \
        } \
        if(!currentDistance || !*best_dist) { return; } \
        /* Calculate the distance from the target point to the splitting dimension of the current node */ \
        T a = A##_static_get_at(tree->ref, node->index + i_dim, tree->len); \
        T b = A##_static_get_at(pt, i_dim, tree->dim); \
        double splittingDistance = b - a; \
        double dx2 = splittingDistance * splittingDistance; \
        /* Traverse the KDTree based on the splitting dimension and the distance to the target */ \
        ssize_t nearerNode; \
        ssize_t furtherNode; \
        if (splittingDistance <= 0) { \
            nearerNode = node->left; \
            furtherNode = node->right; \
        } else { \
            nearerNode = node->right; \
            furtherNode = node->left; \
        } \
        if(++i_dim >= tree->dim) i_dim = 0; \
        /* Search the nearest point in the nearer subtree */ \
        A##_static_nearest(tree, nearerNode, pt, i_dim, best, best_dist); \
        /* Search the nearest point in the further subtree if necessary */ \
        if(dx2 >= *best_dist) { return; } \
        A##_static_nearest(tree, furtherNode, pt, i_dim, best, best_dist); \
    }

#define KDTREE_IMPLEMENT_NEAREST(N, A, T); \
    ssize_t A##_nearest(N *tree, T *pt, double *dist) { \
        assert(tree); \
        assert(pt); \
        double temp_dist = 0; \
        if(!dist) dist = &temp_dist; \
        *dist = INFINITY; \
        ssize_t i = -1; \
        A##_static_nearest(tree, tree->root, pt, 0, &i, dist); \
        ssize_t result = i >= 0 ? kdtree_buckets_get_at(&tree->buckets, i)->index : -1; \
        return result; \
    }

#define KDTREE_IMPLEMENT_FREE(N, A, T) \
    void A##_free(N *tree ) { \
        assert(tree); \
        kdtree_buckets_free(&tree->buckets); \
        memset(tree, 0, sizeof(*tree)); \
    }


#define KDTREE_H
#endif


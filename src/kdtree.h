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
#include <stdbool.h>
#include <math.h> /* INFINITY */

//#include "vec.h"
#include <rlc/array.h>
#include <rlc/err.h>

#define KDTREE_SWAP(x,y)   {ssize_t t = x; x = y; y = t; }
#define KDTREE_DEBUG    1

typedef struct KDTreeNode {
    ssize_t left;
    ssize_t right;
    size_t index;
    bool mark;
} KDTreeNode;

//VEC_INCLUDE(KDTreeBuckets, kdtree_buckets, KDTreeNode, BY_REF);

/*
 * N = name of the kdtree struct
 * A = abbreviation of the kdtree functions
 * T = name of the type struct
 */

#define KDTREE_INCLUDE(N, A, T) \
    typedef struct N { \
        KDTreeNode *buckets; \
        T* ref; \
        size_t len;   /* length of ref array */ \
        size_t dim;   /* count of dimensions */ \
        size_t stride; \
        ssize_t root; /* root returned from create */ \
    } N; \
    \
    int A##_create(N *tree , T *ref, size_t len, size_t dim, size_t offset, size_t stride); \
    ssize_t A##_nearest(N *tree , T *pt, double *squared_dist, bool mark); \
    ssize_t A##_range(N *tree, T *pt, double squared_dist, bool mark, size_t *pts, size_t len); \
    void A##_mark_clear(N *tree); \
    void A##_free(N *tree ); \


#define KDTREE_IMPLEMENT(N, A, T) \
    KDTREE_IMPLEMENT_STATIC_GET_AT(N, A, T); \
    KDTREE_IMPLEMENT_STATIC_MEDIAN(N, A, T); \
    KDTREE_IMPLEMENT_STATIC_CREATE(N, A, T); \
    KDTREE_IMPLEMENT_CREATE(N, A, T); \
    KDTREE_IMPLEMENT_STATIC_DISTANCE(N, A, T); \
    KDTREE_IMPLEMENT_STATIC_NEAREST(N, A, T); \
    KDTREE_IMPLEMENT_NEAREST(N, A, T); \
    KDTREE_IMPLEMENT_STATIC_RANGE(N, A, T); \
    KDTREE_IMPLEMENT_RANGE(N, A, T); \
    KDTREE_IMPLEMENT_CLEAR_MARK(N, A, T); \
    KDTREE_IMPLEMENT_FREE(N, A, T); \

#define KDTREE_IMPLEMENT_STATIC_GET_AT(N, A, T) \
    T A##_static_get_at(T *ref, size_t index, size_t len) { \
        if(KDTREE_DEBUG) { \
            ASSERT(index < len, "accessing index %zu / len %zu", index, len); \
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
            size_t i_pivot = tree->buckets[md].index; \
            T pivot = A##_static_get_at(tree->ref, i_pivot + i_dim, tree->len); \
            KDTREE_SWAP(tree->buckets[md].index, tree->buckets[iE - 1].index); \
            store = i0; \
            for(p = i0; p < iE - 1; p++) { \
                size_t i_p = tree->buckets[p].index; \
                T p_x = A##_static_get_at(tree->ref, i_p + i_dim, tree->len); \
                if(p_x < pivot) { \
                    if(p != store) { \
                        KDTREE_SWAP(tree->buckets[p].index, tree->buckets[store].index); \
                    } \
                    store++; \
                    assert(store < array_len(tree->buckets)); \
                } \
            } \
            KDTREE_SWAP(tree->buckets[store].index, tree->buckets[iE - 1].index); \
            /* median has duplicate values */ \
            size_t i_s = tree->buckets[store].index; \
            T v_s = A##_static_get_at(tree->ref, i_s + i_dim, tree->len); \
            size_t i_m = tree->buckets[md].index; \
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
            KDTreeNode *n = array_it(tree->buckets, m); \
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
        tree->dim = dim; \
        if(!stride) stride = dim; \
        tree->stride = stride; \
        for(size_t i = offset; i < len; i += stride) { \
            array_push(tree->buckets, (KDTreeNode){.index = i}); \
        } \
        tree->len = array_len(tree->buckets) * tree->dim; \
        tree->root = A##_static_create(tree, 0, array_len(tree->buckets), 0); \
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
    static inline void A##_static_nearest(N* tree, ssize_t root, T* pt, size_t i_dim, ssize_t *best, double *best_dist, bool mark) { \
        if(root < 0) return; \
        /* Get the current node from the KDTree */ \
        KDTreeNode* node = array_it(tree->buckets, root); \
        /*printf("node indx !! %zi\n", node->index);*/ \
        /* Calculate the distance from the target point to the current node */ \
        double current_distance = A##_static_distance(tree->dim, pt, &(tree->ref[node->index])); \
        if(((mark && !node->mark) || !mark) && (*best < 0 || current_distance < *best_dist)) { \
            *best = root; \
            *best_dist = current_distance; \
        } \
        if(!current_distance || !*best_dist) { return; } \
        /* Calculate the distance from the target point to the splitting dimension of the current node */ \
        T a = A##_static_get_at(tree->ref, node->index + i_dim, tree->len); \
        T b = A##_static_get_at(pt, i_dim, tree->dim); \
        double splitting_dist = b - a; \
        double dx2 = splitting_dist * splitting_dist; \
        /* Traverse the KDTree based on the splitting dimension and the distance to the target */ \
        ssize_t nearer_node; \
        ssize_t further_node; \
        if (splitting_dist <= 0) { \
            nearer_node = node->left; \
            further_node = node->right; \
        } else { \
            nearer_node = node->right; \
            further_node = node->left; \
        } \
        if(++i_dim >= tree->dim) i_dim = 0; \
        /* Search the nearest point in the nearer subtree */ \
        A##_static_nearest(tree, nearer_node, pt, i_dim, best, best_dist, mark); \
        /* Search the nearest point in the further subtree if necessary */ \
        if(dx2 >= *best_dist) { return; } \
        A##_static_nearest(tree, further_node, pt, i_dim, best, best_dist, mark); \
    }

#define KDTREE_IMPLEMENT_NEAREST(N, A, T); \
    ssize_t A##_nearest(N *tree, T *pt, double *squared_dist, bool mark) { \
        assert(tree); \
        assert(pt); \
        double temp_dist = 0; \
        if(!squared_dist) squared_dist = &temp_dist; \
        *squared_dist = INFINITY; \
        ssize_t i = -1; \
        A##_static_nearest(tree, tree->root, pt, 0, &i, squared_dist, mark); \
        KDTreeNode *node = array_it(tree->buckets, i); \
        ssize_t result = i >= 0 ? node->index : -1; \
        node->mark |= (bool)mark; \
        return result; \
    }

#define KDTREE_IMPLEMENT_STATIC_RANGE(N, A, T) \
    static inline int A##_static_range(N* tree, ssize_t root, T *pt, size_t *pts, size_t len, ssize_t *i, size_t i_dim, double range_dist, bool mark) { \
        if(root < 0) return 0; \
        /* Get the current node from the KDTree */ \
        KDTreeNode* node = array_it(tree->buckets, root); \
        /*printf("node indx %zi\n", node->index);*/ \
        T a = A##_static_get_at(tree->ref, node->index + i_dim, tree->len); \
        /* Calculate the distance from the target point to the current node */ \
        double current_distance = A##_static_distance(tree->dim, pt, &(tree->ref[node->index])); \
        if(((mark && !node->mark) || !mark) && (current_distance < range_dist)) { \
            if(*i >= len) { \
                return -1; \
            } \
            node->mark |= (bool)mark; \
            if(pts) pts[*i] = node->index; \
            (*i)++; \
        } /* else { return 0; } */ \
        /* Calculate the distance from the target point to the splitting dimension of the current node */ \
        T b = A##_static_get_at(pt, i_dim, tree->dim); \
        double splitting_dist = b - a; \
        double dx2 = splitting_dist * splitting_dist; \
        /* Traverse the KDTree based on the splitting dimension and the distance to the target */ \
        ssize_t nearer_node; \
        ssize_t further_node; \
        if (splitting_dist <= 0) { \
            nearer_node = node->left; \
            further_node = node->right; \
        } else { \
            nearer_node = node->right; \
            further_node = node->left; \
        } \
        if(++i_dim >= tree->dim) i_dim = 0; \
        /* Search the nearest point in the nearer subtree */ \
        int result = A##_static_range(tree, nearer_node, pt, pts, len, i, i_dim, range_dist, mark); \
        /* Search the nearest point in the further subtree if necessary */ \
        if(dx2 >= range_dist || result < 0) { return result; } \
        result = A##_static_range(tree, further_node, pt, pts, len, i, i_dim, range_dist, mark); \
        return result; \
    }

#define KDTREE_IMPLEMENT_RANGE(N, A, T); \
    ssize_t A##_range(N *tree, T *pt, double squared_dist, bool mark, size_t *pts, size_t len) { \
        assert(tree); \
        assert(pt); \
        ssize_t used = 0; \
        ssize_t result = (ssize_t)A##_static_range(tree, tree->root, pt, pts, len, &used, 0, squared_dist, mark); \
        return result < 0 ? result : used; \
    }

#define KDTREE_IMPLEMENT_CLEAR_MARK(N, A, T); \
    void A##_mark_clear(N *tree) { \
        assert(tree); \
        size_t len = array_len(tree->buckets); \
        for(size_t i = 0; i < len; i++) { \
            KDTreeNode *node = array_it(tree->buckets, i); \
            node->mark = false; \
        } \
    }

#define KDTREE_IMPLEMENT_FREE(N, A, T) \
    void A##_free(N *tree ) { \
        assert(tree); \
        array_free(tree->buckets); \
        memset(tree, 0, sizeof(*tree)); \
    }


#define KDTREE_H
#endif


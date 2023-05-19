#include "kdtree.h"

VEC_IMPLEMENT(Vec3d, vec3d, Point3d, BY_VAL, 0);
VEC_IMPLEMENT(Vec1d, vec1d, double, BY_VAL, 0);
VEC_IMPLEMENT(KDTreeBuckets, kdtree_buckets, KDTreeNode, BY_REF, 0);

#define SWAP(x,y)   {double t = x; x = y; y = t; }

KDTREE_IMPLEMENT(KDTrD, kdtrd, double, Vec1d, vec1d);

void vec3d_print(Vec3d *vec)
{
    for(size_t i = 0; i < vec->len; i++) {
        //printf("[%zu] %.2f %.2f %.2f\n", i, vec->items[i].x[0], vec->items[i].x[1], vec->items[i].x[2]);
    }
}

void buckets_print(KDTree *kdt)
{
    for(size_t i = 0; i < kdt->buckets.len; i++) {
        size_t j = kdt->buckets.items[i]->index;
        printf("[%zu] %zu -> %.2f %.2f %.2f %2zi <> %2zi\n", i, j, kdt->arr->items[j].x[0], kdt->arr->items[j].x[1], kdt->arr->items[j].x[2], kdt->buckets.items[i]->left, kdt->buckets.items[i]->right);
    }
}









static inline ssize_t kdtree_static_median(KDTree *kdt, size_t i0, size_t iE, size_t i_dim)
{
    assert(kdt);
    if(iE <= i0) return -1LL;
    if(iE == i0 + 1) return i0;
    size_t md = i0 + (iE - i0) / 2;
    size_t store = 0;
    size_t p = 0;
    for(;;) {
        size_t i_pivot = kdtree_buckets_get_at(&kdt->buckets, md)->index;
        double pivot = vec3d_get_at(kdt->arr, i_pivot).x[i_dim];
        SWAP(kdt->buckets.items[md]->index, kdt->buckets.items[iE - 1]->index);
        store = i0;
        for(p = i0; p < iE; p++) {
            size_t i_p = kdtree_buckets_get_at(&kdt->buckets, p)->index;
            double p_x = vec3d_get_at(kdt->arr, i_p).x[i_dim];
            if(p_x < pivot) {
                if(p != store) {
                    SWAP(kdt->buckets.items[p]->index, kdt->buckets.items[store]->index);
                }
                store++;
            }
        }
        SWAP(kdt->buckets.items[store]->index, kdt->buckets.items[iE - 1]->index);
        /* median has duplicate values */
        size_t i_s = kdtree_buckets_get_at(&kdt->buckets, store)->index;
        double v_s = vec3d_get_at(kdt->arr, i_s).x[i_dim];
        size_t i_m = kdtree_buckets_get_at(&kdt->buckets, md)->index;
        double v_m = vec3d_get_at(kdt->arr, i_m).x[i_dim];
        if(v_s == v_m) { return md; }
        if(store > md) iE = store;
        else i0 = store;
    }
    return 0;
}


static inline ssize_t kdtree_static_create(KDTree *kdt, size_t i0, size_t iE, size_t i_dim)
{
    if(!iE) return -1LL;
    ssize_t m = kdtree_static_median(kdt, i0, iE, i_dim);
    if(m >= 0) {
        i_dim = (i_dim + 1) % kdt->dim;
        KDTreeNode *n = kdtree_buckets_get_at(&kdt->buckets, m);
        n->left = kdtree_static_create(kdt, i0, m, i_dim);
        n->right = kdtree_static_create(kdt, m + 1, iE, i_dim);
    }
    return m;
}


int kdtree_create(KDTree *kdt, Vec3d *arr)
{
    assert(kdt);
    assert(kdt->dim);
    assert(arr);
    kdt->arr = arr;
    for(size_t i = 0; i < arr->len; i++) {
        kdtree_buckets_push_back(&kdt->buckets, &(KDTreeNode){.index = i});
    }

#if 0
    ssize_t m = kdtree_static_median(kdt, 0, kdt->buckets.len, 0);
    if(m >= 0) {
    }
    printf("median index %zi\n", m);
#endif
    kdt->root = kdtree_static_create(kdt, 0, kdt->buckets.len, 0);

    return 0;
}




/* FINDING */


static inline double kdtree_static_dist(KDTree *kdt, ssize_t index, Point3d *pt)
{
    size_t dim = kdt->dim;
    double d = 0;
    Point3d a = vec3d_get_at(kdt->arr, index);
    while(dim--) {
        double t = a.x[dim] - pt->x[dim];
        d += t * t;
    }
    return d;
}


static inline void kdtree_static_nearest(KDTree *kdt, ssize_t root, Point3d *pt, size_t i_dim, ssize_t *best, double *dist)
{
    assert(kdt);
    assert(best);
    assert(dist);
    if(root < 0) return;
    double d = kdtree_static_dist(kdt, root, pt);
    Point3d x = vec3d_get_at(kdt->arr, root);
    double dx = x.x[i_dim] - pt->x[i_dim];
    double dx2 = dx * dx;
    if(!*best || d < *dist) {
        *dist = d;
        *best = root;
    }
    /* if chance of exact match is high */
    if(!*dist) return;
    if(++i_dim >= kdt->dim) i_dim = 0;
    KDTreeNode *r = kdtree_buckets_get_at(&kdt->buckets, root);
    kdtree_static_nearest(kdt, dx > 0 ? r->left : r->right, pt, i_dim, best, dist);
    if(dx2 >= *dist) return;
    kdtree_static_nearest(kdt, dx > 0 ? r->right : r->left, pt, i_dim, best, dist);
}


ssize_t kdtree_nearest(KDTree *kdt, Point3d *pt, double *squared_dist)
{
    assert(kdt);
    double dist_temp = 0;
    if(!squared_dist) squared_dist = &dist_temp;
    *squared_dist = INFINITY;
    ssize_t best = -1LL;
    kdtree_static_nearest(kdt, kdt->root, pt, 0, &best, squared_dist);
    return best;
}

#if 0

VEC_IMPLEMENT(KDTreeBuckets, kdtree_buckets, KDTreeNode, BY_VAL, 0);

/* static functions */
void kdtree_node_swap(KDTreeNode *a, KDTreeNode *b);
double kdtree_node_dist(KDTreeNode *a, KDTreeNode *b, size_t dim);
KDTreeNode *kdtree_node_find_median(KDTreeNode *start, KDTreeNode *end, size_t index);
KDTreeNode *kdtree_node_make_tree(KDTreeNode *node, size_t len, size_t i, size_t dim);
void kdtree_node_nearest(KDTreeNode *root, KDTreeNode *find, size_t i, size_t dim, KDTreeNode **best, double *best_dist);



double kdtree_node_dist(KDTreeNode *a, KDTreeNode *b, size_t dim)
{
    assert(dim);
    assert(dim <= KDT_MAX_DIM);
    double d = 0;
    while(dim--) {
        double t = a->val[dim] - b->val[dim];
        d += t * t;
    }
    printf("dist %f\n", d);
    return d;
}

void kdtree_node_nearest(KDTreeNode *root, KDTreeNode *find, size_t i, size_t dim, KDTreeNode **best, double *best_dist)
{
    assert(dim);
    assert(dim <= KDT_MAX_DIM);
    if(!root) return;
    //printf("root left %p, right %p\n", root->left, root->right);
    double d = kdtree_node_dist(root, find, dim);
    double dx = root->val[i] - find->val[i];
    double dx2 = dx * dx;
    if(!*best || d < *best_dist) {
        *best_dist = d;
        *best = root;
    }
    /* if chance of exact match is high */
    //printf("best_dist %f\n", *best_dist);
    if(!*best_dist) return;
    if(++i >= dim) i = 0;
    //printf("dx %f, root %p, left %p right %p, i %zu, dim %zu\n", dx, root, root->left, root->right, i, dim);
    kdtree_node_nearest(dx > 0 ? root->left : root->right, find, i, dim, best, best_dist);
    if(dx2 >= *best_dist) return;
    //printf("dx %f, root %p, left %p right %p, i %zu, dim %zu\n", dx, root, root->left, root->right, i, dim);
    kdtree_node_nearest(dx > 0 ? root->right : root->left, find, i, dim, best, best_dist);
}

void kdtree_node_swap(KDTreeNode *a, KDTreeNode *b)
{
    KDTreeValue tmp;
    printf("\nswap %.0f %.f <> %.0f %.0f\n", a->val[0], a->val[1], b->val[0], b->val[1]);
    memcpy(tmp, a->val, sizeof(tmp));
    memcpy(a->val, b->val, sizeof(tmp));
    memcpy(b->val, tmp, sizeof(tmp));
}


/* see quicksort method */
KDTreeNode *kdtree_node_find_median(KDTreeNode *start, KDTreeNode *end, size_t index)
{
    printf(",");
    if(end <= start) return 0;
    if(end == start + 1) return start;
    KDTreeNode *p, *store, *md = start + (end - start) / 2;
    double pivot;
    printf("l%zui%zu", end - start, index);
    for(;;) {
        pivot = md->val[index];
        printf("p%.0f", pivot);
        kdtree_node_swap(md, end - 1);
        for(store = p = start; p < end; p++) {
            if(p->val[index] < pivot) {
                if(p != store) kdtree_node_swap(p, store);
                store++;
            }
        }
        kdtree_node_swap(store, end - 1);
        assert(store);
        assert(store->val);
        assert(md);
        assert(md->val);
        /* median has duplicate values */
        if(store->val[index] == md->val[index]) return md;
        if(store > md) end = store;
        else start = store;
    }
}

KDTreeNode *kdtree_node_make_tree(KDTreeNode *root, size_t len, size_t i, size_t dim)
{
    KDTreeNode *n = 0;
    assert(dim);
    assert(dim <= KDT_MAX_DIM);
    if(!len) return 0;
    if((n = kdtree_node_find_median(root, root + len, i))) {
        i = (i + 1) % dim;
        n->left = kdtree_node_make_tree(root, n - root, i, dim);
        n->right = kdtree_node_make_tree(root + 1, root + len - (n + 1), i, dim);
    }
    printf(".");
    //printf("%.2f %.2f -> left %p ", n->val[0], n->val[1], n->left);
    //if(n->left) printf("(%.2f %.2f), ", n->left->val[0], n->left->val[1]);
    //printf("right %p ", n->right);
    //if(n->right) printf("(%.2f %.2f)", n->right->val[0], n->right->val[1]);
    //printf("\n");
    //printf("[%p]", n);
    return n;
}


/* PUBLIC FUNCTION IMPLEMENTATION */

void kdtree_sort(KDTree *kdt)
{
    assert(kdt);
    assert(kdt->dim);
    assert(kdt->dim <= KDT_MAX_DIM);
    assert(kdt->buckets.items);
    kdt->root = kdtree_node_make_tree(kdt->buckets.items, kdt->buckets.len, 0, kdt->dim);
    //printf("root left %p, right %p\n", kdt->root->left, kdt->root->right);
}

size_t kdtree_nearest(KDTree *kdt, KDTreeValue find, double *best_dist)
{
    assert(kdt);
    assert(kdt->dim);
    assert(kdt->dim <= KDT_MAX_DIM);
    assert(best_dist);
    KDTreeNode test = {0};
    KDTreeNode tempbase = {0};
    KDTreeNode *temp = &tempbase;
    *best_dist = INFINITY;
    memcpy(&test.val, &find, sizeof(test.val) * kdt->dim);
    kdtree_node_nearest(kdt->root, &test, 0, kdt->dim, &temp, best_dist);
    size_t index = temp - kdt->buckets.items;
    return index;
}


#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#include "kdtree.h"

#define RAND_DOUBLE     ((double)rand() / (double)RAND_MAX)

#define LOG_DO          1
#define LOG(...)        if(LOG_DO) { printf(__VA_ARGS__); }


void vec1d_print_n(Vec1d *vec, size_t i0, size_t n, char *end)
{
    LOG("(");
    for(size_t i = 0; i < n; i++) {
        LOG("%.2f, ", vec1d_get_at(vec, i0+i));
    }
    LOG("\b\b)%s", end);
}

void kdtrd_print(KDTrD *kdt, ssize_t root, size_t spaces)
{
    if(root < 0) return;
    printf("%*s%zu ", (int)spaces, "", kdt->buckets.items[root]->index);
    vec1d_print_n(kdt->ref, kdt->buckets.items[root]->index, kdt->dim, "\n");
    kdtrd_print(kdt, kdt->buckets.items[root]->left, spaces + 1);
    kdtrd_print(kdt, kdt->buckets.items[root]->right, spaces + 1);
}

int main(void)
{
    srand(time(0));


#if 1
    KDTrD tree = {0};
    Vec1d arr = {0};
    Vec1d find = {0};

    size_t dims = 3;
    size_t n = 1000000;
    size_t searches = 10000;
    /* create random array to search on */
    LOG("creating random array (%zux%zu)\n", n, dims);
    for(size_t i = 0; i < n * dims; i++) {
        vec1d_push_back(&arr, RAND_DOUBLE);
    }
    /* create kdtree */
    LOG("creating kdtree of array (%zux%zu)\n", arr.len / dims, dims);
    //printf("make...\n");
    kdtrd_create(&tree, &arr, dims);
    //printf("search...\n");
    for(size_t i = 0; i < searches; i++) {
        LOG("[%7zu] ", i);
        /* create random point to find nearest */
        for(size_t i = 0; i < dims; i++) {
            vec1d_push_back(&find, RAND_DOUBLE);
        }
        LOG("find : ");
        vec1d_print_n(&find, 0, dims, " ");
        double sqrd_dist;
        /* search */
        ssize_t nearest = kdtrd_nearest(&tree, &find, &sqrd_dist);
        LOG("... found [%9zu] ", nearest);
        vec1d_print_n(&arr, nearest, dims, "");
        LOG(" ± √%.5f\n", sqrd_dist);
        vec1d_clear(&find);
    }
    //printf("done\n");

    kdtrd_free(&tree);
    vec1d_free(&arr);
    vec1d_free(&find);
#endif

#if 0
    KDTrD tree = {0};
    Vec1d arr = {0};
    Vec1d find = {0};

    vec1d_push_back(&arr, 2); vec1d_push_back(&arr, 3);
    vec1d_push_back(&arr, 5); vec1d_push_back(&arr, 4);
    vec1d_push_back(&arr, 9); vec1d_push_back(&arr, 6);
    vec1d_push_back(&arr, 4); vec1d_push_back(&arr, 7);
    vec1d_push_back(&arr, 8); vec1d_push_back(&arr, 1);
    vec1d_push_back(&arr, 7); vec1d_push_back(&arr, 2);
    //vec1d_push_back(&arr, 9); vec1d_push_back(&arr, 2); /* TODO why doesn't this get found? */

    kdtrd_create(&tree, &arr, 2);
    //kdtrd_print(&tree, tree.root, 0);

    vec1d_push_back(&find, 9); vec1d_push_back(&find, 2);
    printf("find (%.2f %.2f)\n", vec1d_get_at(&find, 0), vec1d_get_at(&find, 1));
    double sqrd_dist;
    ssize_t nearest = kdtrd_nearest(&tree, &find, &sqrd_dist);
    printf("nearest %zu, %.5fΔ² (%.2f %.2f)\n", nearest, sqrd_dist, vec1d_get_at(&arr, nearest), vec1d_get_at(&arr, nearest + 1));
#endif


#if 0
    KDTree tree2 = {0};
    tree2.dim = 2;

    Vec3d arr2 = {0};
    vec3d_push_back(&arr2, (Point3d){.x = {2, 3}});
    vec3d_push_back(&arr2, (Point3d){.x = {5, 4}});
    vec3d_push_back(&arr2, (Point3d){.x = {9, 6}});
    vec3d_push_back(&arr2, (Point3d){.x = {4, 7}});
    vec3d_push_back(&arr2, (Point3d){.x = {8, 1}});
    vec3d_push_back(&arr2, (Point3d){.x = {7, 2}});
    vec3d_push_back(&arr2, (Point3d){.x = {9, 2}});

    //for(size_t i = 0; i < 1000000; i++) {
    //    vec3d_push_back(&arr2, (Point3d){.x = {RAND_DOUBLE, RAND_DOUBLE, RAND_DOUBLE}});
    //}

    //vec3d_print(&arr2);

    LOG("make tree\n");
    printf("make2...\n");
    kdtree_create(&tree2, &arr2);
    printf("search2...\n");
    kdtree_create(&tree2, &arr2);
    //buckets_print(&tree);

    Point3d pt = {.x = {9, 2}};
    double dist = 0;
    ssize_t nearest = kdtree_nearest(&tree2, &pt, &dist);
    Point3d np = vec3d_get_at(&arr2, nearest);
    LOG("nearest %-10zi (%.2f %.2f) squared_dist %.7f\n", nearest, np.x[0], np.x[1], dist);

    //for(size_t i = 0; i < 100000; i++) {
    //    double dist = 0;
    //    Point3d pt = {.x = {RAND_DOUBLE, RAND_DOUBLE, RAND_DOUBLE}};
    //    LOG("search (%.2f %.2f %.2f) ... ", pt.x[0], pt.x[1], pt.x[2]);
    //    ssize_t nearest = kdtree_nearest(&tree2, &pt, &dist);

    //    Point3d np = vec3d_get_at(&arr2, nearest);
    //    LOG("nearest %-10zi (%.2f %.2f, %.2f) squared_dist %.7f\n", nearest, np.x[0], np.x[1], np.x[2], dist);
    //}
    //printf("done2\n");
#endif

#if 0
    kdtree_buckets_push_back(&tree.buckets, (KDTreeNode){.val = {2, 3}});
    kdtree_buckets_push_back(&tree.buckets, (KDTreeNode){.val = {5, 4}});
    kdtree_buckets_push_back(&tree.buckets, (KDTreeNode){.val = {9, 6}});
    kdtree_buckets_push_back(&tree.buckets, (KDTreeNode){.val = {4, 7}});
    kdtree_buckets_push_back(&tree.buckets, (KDTreeNode){.val = {8, 1}});
    kdtree_buckets_push_back(&tree.buckets, (KDTreeNode){.val = {7, 2}});

    for(size_t i = 0; i < tree.buckets.len; i++) {
        KDTreeNode pt = kdtree_buckets_get_at(&tree.buckets, i);
        printf("[%zu] %.2f %.2f %2zi <> %2zi\n", i, pt.val[0], pt.val[1], pt.left ? pt.left - tree.buckets.items : -1, pt.right ? pt.right - tree.buckets.items : -1);
        //printf("[%zu] %.2f %.2f %.2f\n", i, pt.val[0], pt.val[1], pt.val[2]);
    }
    printf("NOW SORT\n");

    //for(size_t i = 0; i < 10; i++) {
    //    KDTreeNode n = {0};
    //    n.val[0] = RAND_DOUBLE;
    //    n.val[1] = RAND_DOUBLE;
    //    //n.val[2] = RAND_DOUBLE;
    //    kdtree_buckets_push_back(&tree.buckets, n);
    //}

    kdtree_sort(&tree);

#if 1
    printf("\nroot %zu\n", tree.root - tree.buckets.items);
    for(size_t i = 0; i < tree.buckets.len; i++) {
        KDTreeNode pt = kdtree_buckets_get_at(&tree.buckets, i);
        printf("[%zu] %.2f %.2f %2zi <> %2zi\n", i, pt.val[0], pt.val[1], pt.left ? pt.left - tree.buckets.items : -1, pt.right ? pt.right - tree.buckets.items : -1);
        //printf("[%zu] %.2f %.2f %.2f\n", i, pt.val[0], pt.val[1], pt.val[2]);
    }
#endif

    KDTreeValue find = {9, 2};
    double best_dist = 0;
    printf(">> find %.2f %.2f\n", find[0], find[1]);
    //printf(">> find %.2f %.2f %.2f\n", find[0], find[1], find[2]);
    size_t index = kdtree_nearest(&tree, find, &best_dist);
    double *found = kdtree_buckets_get_at(&tree.buckets, index).val;
    printf("[%zu] found %.2f %.2f, dist %.2f\n", index, found[0], found[1], best_dist);
    //printf("[%zu] found %.2f %.2f %.2f, dist %.2f\n", index, found[0], found[1], found[2], best_dist);
#endif

    return 0;
}




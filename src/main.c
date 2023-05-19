#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "kdtree.h"

#define rand_pt     ((double)rand() / (double)RAND_MAX)


int main(void)
{
    srand(time(0));

    KDTrD tree = {0};
    Vec1d arr = {0};
    vec1d_push_back(&arr, 2); vec1d_push_back(&arr, 3);
    vec1d_push_back(&arr, 5); vec1d_push_back(&arr, 4);
    vec1d_push_back(&arr, 9); vec1d_push_back(&arr, 6);
    vec1d_push_back(&arr, 4); vec1d_push_back(&arr, 7);
    vec1d_push_back(&arr, 8); vec1d_push_back(&arr, 1);
    vec1d_push_back(&arr, 7); vec1d_push_back(&arr, 2);

    kdtrd_create(&tree, &arr, 2);

    Vec1d find = {0};
    vec1d_push_back(&find, 9); vec1d_push_back(&find, 2);
    printf("find (%.2f %.2f)\n", vec1d_get_at(&find, 0), vec1d_get_at(&find, 1));
    double sqrd_dist;
    ssize_t nearest = kdtrd_nearest(&tree, &find, &sqrd_dist);
    printf("nearest %zu, %.5fΔ² (%.2f %.2f)\n", nearest, sqrd_dist, vec1d_get_at(&arr, nearest), vec1d_get_at(&arr, nearest + 1));


#if 0
    KDTree tree = {0};
    tree.dim = 3;

    Vec3d arr = {0};
    //vec3d_push_back(&arr, (Point3d){.x = {2, 3}});
    //vec3d_push_back(&arr, (Point3d){.x = {5, 4}});
    //vec3d_push_back(&arr, (Point3d){.x = {9, 6}});
    //vec3d_push_back(&arr, (Point3d){.x = {4, 7}});
    //vec3d_push_back(&arr, (Point3d){.x = {8, 1}});
    //vec3d_push_back(&arr, (Point3d){.x = {7, 2}});

    for(size_t i = 0; i < 1000000; i++) {
        vec3d_push_back(&arr, (Point3d){.x = {rand_pt, rand_pt, rand_pt}});
    }

    vec3d_print(&arr);

    printf("make tree\n");
    kdtree_create(&tree, &arr);
    //buckets_print(&tree);

    for(size_t i = 0; i < 1000000; i++) {
        double dist = 0;
        Point3d pt = {.x = {rand_pt, rand_pt, rand_pt}};
        printf("search (%.2f %.2f %.2f) ... ", pt.x[0], pt.x[1], pt.x[2]);
        ssize_t nearest = kdtree_nearest(&tree, &pt, &dist);

        Point3d np = vec3d_get_at(&arr, nearest);
        printf("nearest %-10zi (%.2f %.2f, %.2f) squared_dist %.7f\n", nearest, np.x[0], np.x[1], np.x[2], dist);
    }
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
    //    n.val[0] = rand_pt;
    //    n.val[1] = rand_pt;
    //    //n.val[2] = rand_pt;
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




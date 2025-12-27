#include "kdtrd.h"

KDTREE_IMPLEMENT(KDTrD, kdtrd, double);

void vecD_print_n(double *vec, size_t i0, size_t n, char *end)
{
    printf("(");
    for(size_t i = 0; i < n; i++) {
        printf("%.2f, ", vec[i0+i]);
    }
    printf("\b\b)%s", end);
}

void kdtrd_print(KDTrD *kdt, ssize_t root, size_t spaces)
{
    if(root < 0) return;
    printf("%*s%zu ", (int)spaces, "", kdt->buckets[root].index);
    vecD_print_n(kdt->ref, kdt->buckets[root].index, kdt->dim, "\n");
    kdtrd_print(kdt, kdt->buckets[root].left, spaces + 1);
    kdtrd_print(kdt, kdt->buckets[root].right, spaces + 1);
}


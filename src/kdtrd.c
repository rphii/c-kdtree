#include "kdtrd.h"

KDTREE_IMPLEMENT(KDTrD, kdtrd, double, Vec1d, vec1d, BY_VAL);

void kdtrd_print(KDTrD *kdt, ssize_t root, size_t spaces)
{
    if(root < 0) return;
    printf("%*s%zu ", (int)spaces, "", kdt->buckets.items[root]->index);
    vec1d_print_n(kdt->ref, kdt->buckets.items[root]->index, kdt->dim, "\n");
    kdtrd_print(kdt, kdt->buckets.items[root]->left, spaces + 1);
    kdtrd_print(kdt, kdt->buckets.items[root]->right, spaces + 1);
}


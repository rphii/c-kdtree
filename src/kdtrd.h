#ifndef KDTRD_H

#include "vec1d.h"
#include "kdtree.h"

KDTREE_INCLUDE(KDTrD, kdtrd, double, Vec1d);

void kdtrd_print(KDTrD *kdt, ssize_t root, size_t spaces);

#define KDTRD_H
#endif

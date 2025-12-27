#ifndef KDTRD_H

#include "vec1d.h"
#include "../src/kdtree.h"

KDTREE_INCLUDE(KDTrD, kdtrd, double);

void kdtrd_print(KDTrD *kdt, ssize_t root, size_t spaces);

#define KDTRD_H
#endif

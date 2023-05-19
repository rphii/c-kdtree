#ifndef VEC1D_H

#define LOG_DO          1
#define LOG(...)        if(LOG_DO) { printf(__VA_ARGS__); }

#include "vec.h"

VEC_INCLUDE(Vec1d, vec1d, double, BY_VAL);

void vec1d_print_n(Vec1d *vec, size_t i0, size_t n, char *end);

#define VEC1D_H
#endif


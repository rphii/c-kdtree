#include "vec1d.h"

VEC_IMPLEMENT(Vec1d, vec1d, double, BY_VAL, 0);

void vec1d_print_n(Vec1d *vec, size_t i0, size_t n, char *end)
{
    LOG("(");
    for(size_t i = 0; i < n; i++) {
        LOG("%.2f, ", vec1d_get_at(vec, i0+i));
    }
    LOG("\b\b)%s", end);
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#include "vec1d.h"
#include "kdtrd.h"

#define RAND_DOUBLE     ((double)rand() / (double)RAND_MAX)

#define LOG_DO          1
#define LOG(...)        if(LOG_DO) { printf(__VA_ARGS__); }


int main(void)
{
    srand(time(0));

    KDTrD tree = {0};
    Vec1d arr = {0};
    Vec1d find = {0};

    size_t dims = 3;
    size_t n = 1000000;
    size_t searches = 100000;
    /* create random array to search on */
    LOG("creating random array (%zux%zu)\n", n, dims);
    for(size_t i = 0; i < n * dims; i++) {
        double val = RAND_DOUBLE;
        vec1d_push_back(&arr, val);
    }
    /* create kdtree */
    LOG("creating kdtree of array (%zux%zu)\n", arr.len / dims, dims);
    //printf("make...\n");
    kdtrd_create(&tree, arr.items, arr.len, dims, 0, 0);
    //printf("search...\n");
    for(size_t i = 0; i < searches; i++) {
        LOG("[%7zu] ", i);
        /* create random point to find nearest */
        for(size_t i = 0; i < dims; i++) {
            double val = RAND_DOUBLE;
            vec1d_push_back(&find, val);
        }
        LOG("find : ");
        vec1d_print_n(&find, 0, dims, " ");
        double sqrd_dist;
        /* search */
        ssize_t nearest = kdtrd_nearest(&tree, find.items, &sqrd_dist);
        LOG("... found [%9zu] ", nearest);
        vec1d_print_n(&arr, nearest, dims, "");
        LOG(" ± √%.5f\n", sqrd_dist);
        vec1d_clear(&find);
    }

    kdtrd_free(&tree);
    vec1d_free(&arr);
    vec1d_free(&find);

    return 0;
}




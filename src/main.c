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

    size_t dims = 10;
    size_t n = 1000000;
    size_t searches = 1000000;
    /* create random array to search on */
    LOG("creating random array (%zux%zu)\n", n, dims);
    for(size_t i = 0; i < n * dims; i++) {
        double val = RAND_DOUBLE;
        vec1d_push_back(&arr, val);
    }
    size_t range_len = 5; /* limit to 5 searches */
    size_t *range = calloc(range_len, sizeof(size_t));
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
        //printf("NEAREST\n");
        ssize_t nearest = kdtrd_nearest(&tree, find.items, &sqrd_dist);
        LOG("... nearest [%9zu] ", nearest);
        vec1d_print_n(&arr, nearest, dims, "");
        LOG(" ± √%.5f\n", sqrd_dist);

        //printf("RANGE\n");
        ssize_t used = kdtrd_range(&tree, find.items, 0.1, range, range_len);
        used = used < 0 ? range_len : used;
        LOG("[%7zu] ", i);
        LOG("find : ");
        vec1d_print_n(&find, 0, dims, " ");
        LOG("... range   x%9zi:  \n", used);
        if(used > 0) for(size_t j = 0; j < used; j++) {
            printf("          [%4zu] ", j);
            vec1d_print_n(&arr, range[j], dims, "");
            double d = 0;
            for(size_t l = 0; l < dims; l++) {
                double dist = arr.items[range[j]+l] - find.items[l];
                //printf("%zu / %zu, %f\n", l, dims, dist);
                d += (dist * dist);
            }
            printf(" ± √%.5f\n", d);
        }
        vec1d_clear(&find);
    }

    kdtrd_free(&tree);
    vec1d_free(&arr);
    vec1d_free(&find);

    return 0;
}




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

void ransac(void)
{
    size_t dims = 2;
    size_t n = 100000;
    size_t n_outlier = 100000;
    size_t n_searches = 50;
    size_t max_search_per = 100000; /* decrease to potentially be faster, but not hit all points in return */

    double y_min = 20;
    double y_max = 50;
    double x_min = 0;
    double x_max = 100;
    double tolerance = 5; /* generation tolerance */
    double dist = 10; /* searching distance */
    double step = 2; /* sqrt(x²+y²); delta of how far to "step" when going over y=mx+b */

    Vec1d arr = {0};

    /* set up array */
    for(size_t i = 0; i < n; i++) {
        /* x value */
        double x_val = (x_max - x_min) * (double)i / (double)n + x_min + tolerance * (RAND_DOUBLE - 0.5) * 2;
        vec1d_push_back(&arr, x_val);
        /* y value */
        double y_val = (y_max - y_min) * (double)i / (double)n + y_min + tolerance * (RAND_DOUBLE - 0.5) * 2;
        vec1d_push_back(&arr, y_val);
        //printf("[%zu] %.1f, %.1f\n", i, x_val, y_val);
    }

    /* add outliers */
    for(size_t i = 0; i < n_outlier; i++) {
        double val = RAND_DOUBLE;
        vec1d_push_back(&arr, val);
    }

    /* generate tree */
    KDTrD tree = {0};
    kdtrd_create(&tree, arr.items, arr.len, dims, 0, 0);

    /* actual ransac'ing */
    double *pt = malloc(sizeof(double) * dims);
    size_t pt1 = 0;
    size_t pt2 = 0;
    size_t best_total = 0;
    size_t best_pt1 = 0;
    size_t best_pt2 = 0;
    for(size_t i = 0; i < n_searches; i++) {
        /* pick random two random points (indices) */
        pt1 = rand() % (arr.len / dims);
        //pt2 == (pt1 + (rand() % (arr.len / dims - 1))) % (arr.len - dims)
        do { pt2 = rand() % (arr.len / dims); } while(pt2 == pt1);
        //printf("points: %zu, %zu\n", pt1, pt2);
        /* go over line from .. until */
        double x1 = arr.items[pt1*dims+0];
        double y1 = arr.items[pt1*dims+1];
        double x2 = arr.items[pt2*dims+0];
        double y2 = arr.items[pt2*dims+1];
        double m = (y2-y1) / (x2-x1);
        double angle = atan2(y2-y1, x2-x1);
        // y = m * x + b
        // b = y - m * x
        // x = (y-b) / m
        double b = y1 - m * x1;
        //printf("pt1 [%.1f, %.1f], pt2 [%.1f, %.1f], m %.2f, b %.2f, angle %.3f\n", x1, y1, x2, y2, m, b, angle);
        /* determine beginning of line */
        double x = x_min;
        double y = m * x + b;
        if(y < y_min) {
            y = y_min;
            x = (y - b) / m;
        } else if(y > y_max) {
            y = y_max;
            x = (y - b) / m;
        }
        /* range check through line */
        size_t total = 0;
        while(!(y < y_min) && !(y > y_max) && !(x < x_min) && !(x > x_max)) {
            pt[0] = x;
            pt[1] = y;
            ssize_t found = kdtrd_range(&tree, pt, dist*dist, true, 0, max_search_per);
            //printf("[%.1f, %.1f], found %zi\n", x, y, found);
            total += found < 0 ? max_search_per : found;
            x += step * cos(angle);
            y += step * sin(angle);
        }
        /* prepare next */
        if(total > best_total) {
            best_total = total;
            printf("new best total %zu\n", best_total);
            best_pt1 = pt1;
            best_pt2 = pt2;
        }
        kdtrd_mark_clear(&tree);
    }

    /* print best stat */ {
    printf("best_total %zu:\n", best_total);
    double x1 = arr.items[best_pt1*dims+0];
    double y1 = arr.items[best_pt1*dims+1];
    double x2 = arr.items[best_pt2*dims+0];
    double y2 = arr.items[best_pt2*dims+1];
    double m = (y2-y1) / (x2-x1);
    double angle = atan2(y2-y1, x2-x1);
    double b = y1 - m * x1;
    printf("m %.2f, b %.2f, angle %.3f\n", m, b, angle);
    }

    /* print ideal stat */ {
    printf("ideal:\n");
    double x1 = x_min;
    double y1 = y_min;
    double x2 = x_max;
    double y2 = y_max;
    double m = (y2-y1) / (x2-x1);
    double angle = atan2(y2-y1, x2-x1);
    double b = y1 - m * x1;
    printf("m %.2f, b %.2f, angle %.3f\n", m, b, angle);
    }

cleanup:
    kdtrd_free(&tree);
}


int main(void)
{
    srand(time(0));

#if 0
    KDTrD tree = {0};
    Vec1d arr = {0};
    Vec1d find = {0};

    size_t dims = 5;
    size_t n = 1000;
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
    bool first = true;
    for(size_t i = 0; i < searches; i++) {
        LOG("[%7zu] ", i);
        /* create random point to find nearest */
        if(first) {
        for(size_t i = 0; i < dims; i++) {
            double val = RAND_DOUBLE;
            vec1d_push_back(&find, val);
        } first = false; }
        LOG("find : ");
        vec1d_print_n(&find, 0, dims, " ");
        double sqrd_dist;
        /* search */
        //printf("NEAREST\n");
        ssize_t nearest = kdtrd_nearest(&tree, find.items, &sqrd_dist, false);
        LOG("... nearest [%9zu] ", nearest);
        vec1d_print_n(&arr, nearest, dims, "");
        LOG(" ± √%.5f\n", sqrd_dist);

        //printf("RANGE\n");
        ssize_t root = 17;
        ssize_t used = kdtrd_range(&tree, find.items, 0.1, false, range, range_len);
        used = used < 0 ? range_len : used;
        LOG("[%7zu] ", i);
        LOG("find : ");
        vec1d_print_n(&find, 0, dims, " ");
        LOG("... range   x%9zi:  \n", used);
        if(used > 0) for(size_t j = 0; j < used; j++) {
            printf("          [%4zu] ", range[j]);
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

    free(range);
    kdtrd_free(&tree);
    vec1d_free(&arr);
    vec1d_free(&find);
#endif

    ransac();

    return 0;
}




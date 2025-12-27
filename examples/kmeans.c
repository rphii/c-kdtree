#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb/stb_image_resize2.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <rlso.h>
#include <rlpw.h>

#include "../src/kdtree.h"
KDTREE_INCLUDE(Kd_dbl, kd_dbl, double);
KDTREE_IMPLEMENT(Kd_dbl, kd_dbl, double);
KDTREE_INCLUDE(Kd_u8, kd_u8, uint8_t);
KDTREE_IMPLEMENT(Kd_u8, kd_u8, uint8_t);

#define DEBUG   0

void u8_normalize(double *out, unsigned char *in, int len) {
    for(size_t i = 0; i < len; ++i) {
        out[i] = (double)in[i] / 255.0;
    }
}

void u8_scale(unsigned char *out, double *in, int len) {
    for(size_t i = 0; i < len; ++i) {
        double val = round((double)in[i] * 255.0);
        out[i] = val > 255 ? 255 : val;
    }
}

void centroid_rand_from_data(double *centroid, double *data, int w, int h, int ch) {
    size_t i = rand() % (w * h);
    for(int j = 0; j < ch; ++j) {
        centroid[j] = data[i * ch + j];
        ASSERT(!isnan(centroid[j]), "do not want nan");
    }
}

void centroids_print(double *centroids, int ch, int n_clusters) {
    So col = SO;
    for(size_t i = 0; i < n_clusters; ++i) {
#if DEBUG
        printf(" [%zu] ", i);
#endif
        Color bg = {0};
        for(size_t j = 0; j < ch; ++j) {
#if DEBUG
            printf(" %8.3f", centroids[i * ch + j]);
#endif
            if(j == 0) bg.r = round(centroids[i * ch + j] * 255);
            if(j == 1) bg.g = round(centroids[i * ch + j] * 255);
            if(j == 2) bg.b = round(centroids[i * ch + j] * 255);
        }

#if 1
#if DEBUG
        printf("   ");
#endif
        so_clear(&col);
        so_fmt_color(&col, bg, SO_COLOR_RGB|SO_COLOR_HEX);
        printf("%.*s", SO_F(col));
#endif

#if DEBUG
        printf("\n");
#else
        printf(" ");
#endif
    }
    so_free(&col);
}

void counts_print(unsigned int *counts, int len) {
#if DEBUG
    printf("counts: ");
    for(int i = 0; i < len; ++i) {
        printf("%u %u%s", i, counts[i], i + 1 < len ? ", " : "");
    }
    printf("\n");
#endif
}

double clamp(double v, double min, double max) {
    if(v < min) return min;
    if(v > max) return max;
    return v;
}

#define DITHER  1

void kmeans_apply(unsigned char *data, int w, int h, int ch, unsigned int n_clusters, uint8_t *centroids) {

    double error;

    Kd_u8 kd = {0};
    kd_u8_create(&kd, centroids, ch * n_clusters, ch, 0, 0);
    for(size_t y = 0; y < h; ++y) {
        for(size_t x = 0; x < w; ++x) {
            size_t i = y * w + x;
            unsigned char *pt = &data[i * ch];
            size_t i_near = kd_u8_nearest(&kd, pt, 0, false);
            unsigned char *ct = &centroids[i_near];
            for(size_t j = 0; j < ch; ++j) {
                /* collect error for dithering */
                error = (double)pt[j] - (double)ct[j];
                /* apply current value */
                pt[j] = ct[j];

#if DITHER
                if(x+1 < w) {
                    uint8_t *p = &data[((y+0) * w + (x+1)) * ch+j];
                    *p = clamp(round((double)*p + error * 7./16.), 0, 255);
                }
                if(x >= 1 && y+1 < h) {
                    uint8_t *p = &data[((y+1) * w + (x-1)) * ch+j];
                    *p = clamp(round((double)*p + error * 3./16.), 0, 255);
                }
                if(y+1 < h) {
                    uint8_t *p = &data[((y+1) * w + (x+0)) * ch+j];
                    *p = clamp(round((double)*p + error * 5./16.), 0, 255);
                }
                if(x+1 < w && y+1 < h) {
                    uint8_t *p = &data[((y+1) * w + (x+1)) * ch+j];
                    *p = clamp(round((double)*p + error * 1./16.), 0, 255);
                }
#endif
            }
        }
    }
    kd_u8_free(&kd);
}

bool kmeans_data(uint8_t *centroids_out, double *data, int w, int h, int ch, unsigned int n_clusters) {
    //kd_dbl_create(&kd, data, w * h * ch, ch, 0, 0);
    //printf("created kdtree, w %u, h %u, ch %u\n", w, h, ch);

    //unsigned int *indices = malloc(sizeof(*indices) * w * h);
    unsigned int *running_counts = malloc(sizeof(*running_counts) * n_clusters);
    double *running_center = malloc(sizeof(*running_center) * ch * n_clusters);
    double *centroids = malloc(sizeof(*centroids) * ch * n_clusters);
    for(size_t j = 0; j < n_clusters; ++j) {
        centroid_rand_from_data(&centroids[ch * j], data, w, h, ch);
    }

    unsigned int max_iteration = 10000;
    bool have_smth = false;

    for(unsigned int it = 0; it < max_iteration; ++it) {

        memset(running_counts, 0, sizeof(*running_counts) * n_clusters);
        memset(running_center, 0, sizeof(*running_center) * ch * n_clusters);
        Kd_dbl kd = {0};
        kd_dbl_create(&kd, centroids, ch * n_clusters, ch, 0, 0);

        /* go figure out all distances */

#if 0
        printff("initial centroids..");
        centroids_print(centroids, ch, n_clusters);
        counts_print(running_counts, n_clusters);
#endif

        for(size_t i = 0; i < w * h; ++i) {
            size_t i_nearest = kd_dbl_nearest(&kd, &data[ch*i], 0, false) / ch;
            ++running_counts[i_nearest];
            for(size_t j = 0; j < ch; ++j) {
                running_center[i_nearest * ch + j] += data[ch * i + j];
            }
#if 0
            printff("%5zu, i_nearest %zu", i, i_nearest);
            centroids_print(running_center, ch, n_clusters);
            counts_print(running_counts, n_clusters);
#endif
            //printf("i %zu, dist %f, near %zu\n", i, dist, i_nearest);
        }
        /* update centers */
        unsigned int changed = false;
        for(size_t k = 0; k < n_clusters; ++k) {
            bool reinit = false;
            for(size_t j = 0; j < ch; ++j) {
                double *centroid_replace = &centroids[k * ch + j];
                if(running_counts[k]) {
                    double centroid_new = running_center[k * ch + j] / (double)running_counts[k];
                    if(*centroid_replace != centroid_new) {
                        changed = true;
                    }
                    ASSERT(!isnan(centroid_new), "do not want nan. old value: %f, new: %f", *centroid_replace, centroid_new);
                    *centroid_replace = centroid_new;
                } else {
                    reinit = true;
                }
            }
            if(reinit) {
                centroid_rand_from_data(&centroids[k * ch], data, w, h, ch);
                changed = true;
            }
        }

        if(!changed) {
#if 1
#if DEBUG
            printff("\nupdated it=%u..", it);
#endif
            for(size_t i = 0; i < ch * n_clusters; ++i) {
                uint8_t ct = round(centroids[i] * 255);
                centroids_out[i] = ct;
            }
            have_smth = true;

            centroids_print(centroids, ch, n_clusters);
            counts_print(running_counts, n_clusters);
#endif
            kd_dbl_free(&kd);
            break;
        } else {
#if DEBUG
            printf("it:%u\r", it);
#endif
        }

        kd_dbl_free(&kd);

        //break;
    }


    //bool converged = false;
    //size_t n_run = 0;
    //double dist = 0;
    //while(!converged) {
    //    for(size_t k = 0; k < n_clusters; ++k) {
    //        kd_dbl_mark_clear(&kd);
    //        for(size_t i = 0; i < w * h; ++i) {
    //            size_t i_nearest = kd_dbl_nearest(&kd, &centroids[ch*k], &dist, true);
    //            printf("run %zu, k %zu, i %zu, dist %f, near %zu\n", n_run, k, i, dist, i_nearest);
    //        }
    //    }
    //    break;
    //    if(n_run++ > 3) break;
    //}

    // size_t max_iter = 20;
    // for(size_t it = 0; it < max_iter; ++it) {
    // }

    free(running_center);
    free(running_counts);
    free(centroids);
    return have_smth;
}

void kmeans_image_file(char *filename, unsigned int n_clusters) {
    int w1, h1, ch, w, h, w2, h2;
    unsigned char *data1 = stbi_load(filename, &w1, &h1, &ch, 0);
    So path_out = so("results");
    so_path_join(&path_out, path_out, so_get_basename(so_l(filename)));
    so_extend(&path_out, so(".png"));
    if(data1) {
#if DEBUG
        printf("read ok: %s\n", filename);
#else
        printf("%s ", filename);
#endif
        w = ceil((double)w1 / 50.0);
        h = ceil((double)h1 / 50.0);
        unsigned char *data = malloc(w * h * ch);
        stbir_resize_uint8_linear(data1, w1, h1, 0, data, w, h, 0, ch);
        double *ddata = malloc(sizeof(ddata) * w * h * ch);
        u8_normalize(ddata, data, w * h * ch);
#if DEBUG
        printf("resize ok\n");
#endif
        uint8_t *centroids = malloc(ch * n_clusters);
        bool ok = kmeans_data(centroids, ddata, w, h, ch, n_clusters);

#if !DEBUG
        printf("\n");
#endif

        if(ok) {
            w2 = ceil((double)w1 / 2.0);
            h2 = ceil((double)h1 / 2.0);
            unsigned char *data2 = malloc(w2 * h2 * ch);
            stbir_resize_uint8_linear(data1, w1, h1, 0, data2, w2, h2, 0, ch);
            kmeans_apply(data2, w2, h2, ch, n_clusters, centroids);
            char *cfileout = so_dup(path_out);
            stbi_write_png(cfileout, w2, h2, ch, data2, 0);
            free(data2);
            free(cfileout);
        }
        free(ddata);
        free(data);

    } else {
        printf("failed opening file: %s\n", filename);
    }
    so_free(&path_out);
    free(data1);
}

#include <unistd.h>

typedef struct Task {
    unsigned int n_clusters;
    char *filename;
} Task;

void *task_kmeans(Pw *pw, bool *quit, void *void_task) {
    Task *task = void_task;
    kmeans_image_file(task->filename, task->n_clusters);
    free(task);
    return 0;
}

int main(int argc, char **argv) {
    if(argc < 3) {
        printf("no number of clusters or no image specified. exiting.\n");
        exit(1);
    }

    long number_of_processors = sysconf(_SC_NPROCESSORS_ONLN);

    Pw pw = {0};
    pw_init(&pw, number_of_processors);
    pw_dispatch(&pw);

    unsigned int n_clusters;
    if(so_as_uint(so_l(argv[1]), &n_clusters, 0)) {
        printf("failed parsing number of clusters: %s. exiting.\n", argv[1]);
        exit(1);
    }

    for(int i = 2; i < argc; ++i) {
        Task *taskdata;
        NEW(Task, taskdata);
        taskdata->filename = argv[i];
        taskdata->n_clusters = n_clusters;
        pw_queue(&pw, task_kmeans, taskdata);
    }

    while(pw_is_busy(&pw)) {
        usleep(1e5);
    }

    return 0;
}


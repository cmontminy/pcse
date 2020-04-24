#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

// Declaration of subroutines
void initialize(float *, long);
void smooth(float *, float *, long, float, float, float);
void count(float *, long, float, long *);

void main() {
    #ifdef _OPENMP
    omp_set_num_threads(8);
    #endif
    // Dummy parallel region
    #pragma omp parallel
    {   
        #ifdef _OPENMP
        printf("This is thread %d\n", omp_get_thread_num());
        #endif
    }

    // Declaration of constants
    const float a = 0.05;
    const float b = 0.1;
    const float c = 0.4;
    const float threshold = 0.1;
    const long array_size = 98306;

    // Declaration of variables
    float *x_array;
    float *y_array;
    long x_below_elements;
    long y_below_elements;
    double num_elements = array_size * array_size;
    double num_in_elements = (array_size - 2) * (array_size - 2);

    struct timespec start, stop;
    double alloc_x_time;
    double alloc_y_time;
    double init_x_time;
    double init_x_time2;
    double smooth_time;
    double count_x_time;
    double count_y_time;


    // Allocation of arrays
    printf("Allocating arrays . . .");
    clock_gettime(CLOCK_MONOTONIC, &start);
    x_array = (float *) malloc(array_size * array_size * sizeof(float));
    clock_gettime(CLOCK_MONOTONIC, &stop);
    alloc_x_time = (stop.tv_sec - start.tv_sec) + (double) (stop.tv_nsec - start.tv_nsec) / 1000000000;

    clock_gettime(CLOCK_MONOTONIC, &start);
    y_array = (float *) malloc(array_size * array_size * sizeof(float));
    clock_gettime(CLOCK_MONOTONIC, &stop);
    alloc_y_time = (stop.tv_sec - start.tv_sec) + (double) (stop.tv_nsec - start.tv_nsec) / 1000000000;
    printf(" OK\n");


    // Initialize x_array
    printf("Initalizing arrays . . .");
    clock_gettime(CLOCK_MONOTONIC, &start);
    initialize(x_array, array_size);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    init_x_time = (stop.tv_sec - start.tv_sec) + (double) (stop.tv_nsec - start.tv_nsec) / 1000000000;
    printf(" OK\n");

    //Initialize x_array part 2
    // printf("Initalizing arrays part 2 . . .");
    // clock_gettime(CLOCK_MONOTONIC, &start);
    // initialize(x_array, array_size);
    // clock_gettime(CLOCK_MONOTONIC, &stop);
    // init_x_time2 = (stop.tv_sec - start.tv_sec) + (double) (stop.tv_nsec - start.tv_nsec) / 1000000000;
    // printf(" OK\n");


    // Smooth x_array to derive y_array
    printf("Smoothing x array . . .");
    clock_gettime(CLOCK_MONOTONIC, &start);
    smooth(x_array, y_array, array_size, a, b, c);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    smooth_time = (stop.tv_sec - start.tv_sec) + (double) (stop.tv_nsec - start.tv_nsec) / 1000000000;
    printf(" OK\n");


    // Count x_array
    printf("Counting x array . . .");
    clock_gettime(CLOCK_MONOTONIC, &start);
    count(x_array, array_size, threshold, &x_below_elements);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    count_x_time = (stop.tv_sec - start.tv_sec) + (double) (stop.tv_nsec - start.tv_nsec) / 1000000000;
    printf(" OK\n");


    // Count y_array
    printf("Counting y array . . .");
    clock_gettime(CLOCK_MONOTONIC, &start);
    count(y_array, array_size, threshold, &y_below_elements);
    clock_gettime(CLOCK_MONOTONIC, &stop);
    count_y_time = (stop.tv_sec - start.tv_sec) + (double) (stop.tv_nsec - start.tv_nsec) / 1000000000;
    printf(" OK\n");


    // Print outputs
    printf("---------- Summary ----------\n");
    printf("%-45s: %3d\n", "Number of elements in a row/column", array_size);
    printf("%-45s: %3d\n", "Number of inner elements in a row/column", array_size - 2);
    printf("%-45s: %3.0f\n", "Total number of elements", num_elements);
    printf("%-45s: %3.0f\n", "Total number of inner elements", num_in_elements);
    printf("%-45s: %3f\n", "Memory (GB) used per array", (sizeof(float) * num_elements) / 1073741824.0);
    printf("%-45s: %.3f\n", "Threshold", threshold);
    printf("%-45s: %.2f %.2f %.2f\n", "Smoothing constants (a, b, c)", a, b, c);

    printf("\n--- Inner Element Information ---\n");
    printf("%-45s: %ld\n", "Number   of elements below threshold (x)", x_below_elements);
    printf("%-45s: %3f\n", "Fraction of elements below threshold (x)", x_below_elements / num_in_elements);
    printf("%-45s: %ld\n", "Number   of elements below threshold (y)", y_below_elements);
    printf("%-45s: %3f\n", "Fraction of elements below threshold (y)", y_below_elements / num_in_elements);

    printf("\n----- Action -----\n");
    printf("%-25s: %.3f\n", "CPU: Alloc-x", alloc_x_time);
    printf("%-25s: %.3f\n", "CPU: Alloc-y", alloc_y_time);
    printf("%-25s: %.3f\n", "CPU: Init-x", init_x_time);
    //printf("%-25s: %.3f\n", "CPU: Init-x 2", init_x_time2);
    printf("%-25s: %.3f\n", "CPU: Smooth", smooth_time);
    printf("%-25s: %.3f\n", "CPU: Count-x", count_x_time);
    printf("%-25s: %.3f\n", "CPU: Count-y", count_y_time);

    #ifdef _OPENMP
    printf("%-25s: %d\n", "OMP: Number of threads", omp_get_max_threads());
    #endif


    // Free memory
    free(x_array);
    free(y_array);
}

void initialize(float *array, long array_size) {
    #pragma omp parallel for collapse(2) schedule(static)
        for (int j = 0; j < array_size; j++) {
            for (int i = 0; i < array_size; i++) {
                array[i + j * array_size] = (float)abs(i % 11 - j % 5) / (i % 7 + j % 3 + 1);
            }
        }
}

void smooth(float *array, float *out_array, long array_size, float a, float b, float c) {
    #pragma omp parallel for collapse(2) schedule(static)
        for (int j = 1; j < array_size - 1; j++) {
            for (int i = 1; i < array_size - 1; i++) {
                out_array[i + j * array_size] = 
                    a * (array[(i - 1) + (j - 1) * array_size] +
                        array[(i - 1) + (j + 1) * array_size] +
                        array[(i + 1) + (j - 1) * array_size] +
                        array[(i + 1) + (j + 1) * array_size]) +
                    b * (array[(i - 1) + j * array_size] +
                        array[(i + 1) + j * array_size] +
                        array[i + (j - 1) * array_size] +
                        array[i + (j + 1) * array_size]) +
                    c * (array[i + j * array_size]);
            }
        }
} 

void count(float *array, long array_size, float threshold, long *below_threshold) {
    long temp = 0;

    #pragma omp parallel for collapse(2) reduction(+:temp) schedule(static)
        for (int j = 1; j < array_size - 1; j++) {
            for (int i = 1; i < array_size - 1; i++) {
                if (array[i + j * array_size] < threshold) {
                    temp++;
                }
            }
        }
    *below_threshold = temp;
}
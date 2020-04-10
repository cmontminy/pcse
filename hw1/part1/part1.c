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
    printf("Summary\n-------\n");
    printf("Number of elements in a row/column       ::   %3d\n", array_size);
    printf("Number of inner elements in a row/column ::   %3d\n", array_size - 2);
    printf("Total number of elements                 ::   %3.0f\n", num_elements);
    printf("Total number of inner elements           ::   %3.0f\n", num_in_elements);
    printf("Memory (GB) used per array               ::   %3f\n", (sizeof(float) * num_elements) / 1073741824.0);
    printf("Threshold                                ::   %.3f\n", threshold);
    printf("Smoothing constants (a, b, c)            ::   %.2f %.2f %.2f\n", a, b, c);

    printf("--- Inner Element Information ---\n");
    printf("Number   of elements below threshold (x) ::   %ld\n", x_below_elements);
    printf("Fraction of elements below threshold (x) ::   %3f\n", x_below_elements / num_in_elements);
    printf("Number   of elements below threshold (y) ::   %ld\n", y_below_elements);
    printf("Fraction of elements below threshold (y) ::   %3f\n", y_below_elements / num_in_elements);

    printf("\n\nAction\n------\n");
    printf("CPU: Alloc-x           ::   %.3f\n", alloc_x_time);
    printf("CPU: Alloc-y           ::   %.3f\n", alloc_y_time);
    printf("CPU: Init-x            ::   %.3f\n", init_x_time);
    printf("CPU: Smooth            ::   %.3f\n", smooth_time);
    printf("CPU: Count-x           ::   %.3f\n", count_x_time);
    printf("CPU: Count-y           ::   %.3f\n", count_y_time);

    #ifdef _OPENMP
    printf("OMP: Number of threads ::   %d\n", omp_get_max_threads());
    #endif


    // Free memory
    free(x_array);
    free(y_array);
}

void initialize(float *array, long array_size) {
    //printf("array size: %d\n", array_size);
    #pragma omp parallel for collapse(2)
        for (long i = 0; i < array_size; i++) {
            for (long j = 0; j < array_size; j++) {
                array[i + j * array_size] = (float)abs(i % 11 - j % 5) / (i % 7 + j % 3 + 1);
            }
        }
}

void smooth(float *array, float *out_array, long array_size, float a, float b, float c) {
    #pragma omp parallel for collapse(2)
        for (long i = 1; i < array_size - 1; i++) {
            for (long j = 1; j < array_size - 1; j++) {
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

    #pragma omp parallel for collapse(2) reduction(+:temp)
        for (long i = 1; i < array_size - 1; i++) {
            for (long j = 1; j < array_size - 1; j++) {
                if (array[i + j * array_size] < threshold) {
                    temp++;
                }
            }
        }
    *below_threshold = temp;
}
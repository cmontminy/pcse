#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BILLION 1000000000L

// Declaration of subroutines
void initialize(float *, size_t);
void smooth(float *, float *, size_t, float, float, float);
void count(float *, size_t, float, float *);

void main() {

    // Declaration of constants
    const float a = 0.05;
    const float b = 0.1;
    const float c = 0.4;
    const float threshold = 0.1;
    const size_t array_size = 81922;

    // Declaration of variables
    float *x_array;
    float *y_array;
    float x_below_elements;
    float y_below_elements;
    double num_elements = array_size * array_size;
    double num_in_elements = (array_size - 2) * (array_size - 2);

    struct timespec start, stop;
    float alloc_x_time;
    float alloc_y_time;
    float init_x_time;
    float smooth_time;
    float count_x_time;
    float count_y_time;

    // Allocation of arrays
    printf("Allocating arrays . . .");
    clock_gettime(CLOCK_MONOTONIC, &start);
    x_array = malloc(array_size * array_size * sizeof(float));
    clock_gettime(CLOCK_MONOTONIC, &stop);
    alloc_x_time = (stop.tv_sec - start.tv_sec) + (double) (stop.tv_nsec - start.tv_nsec) / 1000000000;

    clock_gettime(CLOCK_MONOTONIC, &start);
    y_array = malloc(array_size * array_size * sizeof(float));
    clock_gettime(CLOCK_MONOTONIC, &stop);
    alloc_y_time = (stop.tv_sec - start.tv_sec) + (double) (stop.tv_nsec - start.tv_nsec) / 1000000000;
    printf(" OK\n");
    
    // Initialize x_array
    printf("Initalizing arrays . . .");
    t = clock();
    initialize(x_array, array_size);
    t = clock() - t;
    init_x_time = (float)t / CLOCKS_PER_SEC;
    printf(" OK\n");

    // Smooth x_array to derive y_array
    printf("Smoothing x array . . .");
    t = clock();
    smooth(x_array, y_array, array_size, a, b, c);
    t = clock() - t;
    smooth_time = (float)t / CLOCKS_PER_SEC;
    printf(" OK\n");

    // Count x_array
    printf("Counting x array . . .");
    t = clock();
    count(x_array, array_size, threshold, &x_below_elements);
    t = clock() - t;
    count_x_time = (float)t / CLOCKS_PER_SEC;
    printf(" OK\n");

    // Count y_array
    printf("Counting y array . . .");
    t = clock();
    count(y_array, array_size, threshold, &y_below_elements);
    t = clock() - t;
    count_y_time = (float)t / CLOCKS_PER_SEC;
    printf(" OK\n");

    // Print outputs
    printf("\nSummary\n-------\n");
    printf("Number of elements in a row/column       ::   %3d\n", array_size);
    printf("Number of inner elements in a row/column ::   %3d\n", array_size - 2);
    printf("Total number of elements                 ::   %3.0f\n", num_elements);
    printf("Total number of inner elements           ::   %3.0f\n", num_in_elements);
    printf("Memory (GB) used per array               ::   %3f\n", (sizeof(float) * num_elements) / 1073741824.0);
    printf("Threshold                                ::   %3f\n", threshold);
    printf("Smoothing constants (a, b, c)            ::   %3f %f %f\n", a, b, c);

    printf("--- Inner Element Information ---\n");
    printf("Number   of elements below threshold (x) ::   %3.0f\n", x_below_elements);
    printf("Fraction of elements below threshold (x) ::   %3f\n", x_below_elements / num_in_elements);
    printf("Number   of elements below threshold (y) ::   %3.0f\n", y_below_elements);
    printf("Fraction of elements below threshold (y) ::   %3f\n", y_below_elements / num_in_elements);

    printf("\n\nAction\n------\n");
    printf("CPU: Alloc-x           ::   %3f\n", alloc_x_time);
    printf("CPU: Alloc-y           ::   %3f\n", alloc_y_time);
    printf("CPU: Init-x            ::   %3f\n", init_x_time);
    printf("CPU: Smooth            ::   %3f\n", smooth_time);
    printf("CPU: Count-x           ::   %3f\n", count_x_time);
    printf("CPU: Count-y           ::   %3f\n", count_y_time);

    // Free memory
    free(x_array);
    free(y_array);
}

void initialize(float *array, size_t array_size) {
    for (size_t i = 0; i < array_size; i++) {
        for (size_t j = 0; j < array_size; j++) {
            array[i + j * array_size] = (float)abs(i % 11 - j % 5) / (i % 7 + j % 3 + 1);
        }
    }
}

void smooth(float *array, float *out_array, size_t array_size, float a, float b, float c) {
    for (size_t i = 1; i < array_size - 1; i++) {
        for (size_t j = 1; j < array_size - 1; j++) {
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


void count(float *array, size_t array_size, float threshold, float *below_threshold) {
    for (size_t i = 1; i < array_size - 1; i++) {
        for (size_t j = 1; j < array_size - 1; j++) {
            if (array[i + j * array_size] < threshold) {
                *below_threshold = *below_threshold + 1;
            }
        }
    }
}
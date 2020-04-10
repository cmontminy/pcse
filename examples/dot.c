#include <stdio.h>
#include <stdint.h>
#include <omp.h>

// dot product of 2 1d arrays

int main() {
    // set the size n
    const uint32_t SIZE = 50000000; // large enough to force into cache

    printf("Number of elements: %ld\n", 2 * SIZE);
    printf("Size of each array in MB: %f\n", 
        (float) sizeof(float) * (float) SIZE / (1024.0 * 1024.0));

    // allocate memory
    float *x = malloc(SIZE * sizeof(float));
    float *y = malloc(SIZE * sizeof(float));

    if (x == NULL || y == NULL) // ensures mallocs were allocated correctly
        exit(-1);

    // put values within the arrays
    for (uint32_t i = 0; i < SIZE; i++) {
          x[i] = 3;
          y[i] = 3.3333;
    }

    // dot product calculation
    float sum;
    int dot_times = 200;
    double start = omp_get_wtime();

    for (int i = 0; i < dot_times; i++) {
        sum = 0;
        #pragma omp parallel for reduction(+:sum)
        for(uint32_t i = 0; i < SIZE; i++) {
            sum = sum + x[i] * y[i];
        }
    }
    double end = omp_get_wtime();

    // Output calculations:
    // Number of threads used
    int num_threads = omp_get_max_threads();

    // Time taken to execute
    float time = end - start;

    // Number of floating point operations performed
    float total_ops = dot_times * SIZE * 2.0; // one for add, one for mul

    float bytes_read = dot_times * SIZE * sizeof(float) * 2.0;

    // Clock speed of CPUS (GHz)s
    float clock_rate = 2.1;

    // Number of FP operations (FLOPs) performed per clock cycle of CPU
    float ops_per_clock = total_ops / (clock_rate * 1000000000 * time);

    float gb_per_sec = bytes_read / (time * 1024.0 * 1024.0 * 1024.0);

    float peak_gb_per_sec = 120;

    float per_mem_of_peak = 100 * gb_per_sec / peak_gb_per_sec;

    // Number of FP operations (FLOPs) performed per cycle of CPU by a single thread
    float ops_per_clk_per_thread = ops_per_clock / num_threads;

    // Peak (theoretical) flaoting point operations per clock cycle on a single core
    // 2 FPUs/ VPUs per CPU * 16 Single Per FP per AVX INstruction * 2 FUsed Multiply/Add
    float peak_ops_per_clock = 64;

    // Number of CPU cores in a CPU socket
    float num_cores = 24;

    // Number of FP operations a single socket can perform in a clock cycle
    float peak_ops_per_clock_per_skt = peak_ops_per_clock * num_cores;

    // Percentage of peak FLOPs per thread performed by a single thread
    float per_of_peak_ops_per_thd = 100 * ops_per_clk_per_thread / peak_ops_per_clock;

    // Percentage of peak FLOPs per socket performed overall
    float per_of_peak_ops_skt = 100 * ops_per_clock / peak_ops_per_clock_per_skt;

    printf("---------------------Results--------------------\n");
    pritnf("%-31s: %10ld\n", "Number of elements per array", 2 * SIZE);
    printf("%-31s: %10.2f MB\n\n", "Size of each array", (float) sizeof(float) * (float) N / mb);

    printf("%31s: %10.4e\n", "Calculated dot product", sum);
    // Theoretical dot product result
    pritnf("%31s: %10.4e\n\n", "Correct dot product", -1);
    
    printf("%31s: %10d\n", "Times dot product calculated", dot_times);
    
    printf("%31s: %10d\n", "Number of threads", num_threads);
    printf("%31s: %10.3lf s\n\n", "Time taken", time);

    printf("%31s: %10.3e\n", "Total FP operations", total_ops);
    printf("%31s: %10.1f GHz\n", "Clock rate", clock_rate);
    printf("%31s: %10.4f\n", "FLOPs per clock performed", ops_per_clock);
    printf("%31s: %10.4f\n", "FLOPs per clock per thread", ops_per_clk_per_thread);
    printf("%31s: %10.3f %%\n", "Percentage of peak thread FLOPs", per_of_peak_ops_per_thd);
    printf("%31s: %10.3f %%\n", "Percentage of peak socket FLOPs", per_of_peak_ops_skt);
}

// SDS374C OMP Performance Measurement

#include <omp.h>
#include <stdint.h> // For specific-width integers
#include <stdio.h>

int main() {

    const uint32_t N = 50000000;
    const float mb = 1024.0 * 1024.0;

    // Allocate arrays
    float * x = malloc(N * sizeof(float));
    float * y = malloc(N * sizeof(float));

    if(x == NULL) {
        printf("Allocation of array X failed!\n");
        exit(-1);
    }
    if(y == NULL) {
        printf("Allocation of array Y failed!\n");
        exit(-1);
    }

    // Initialize arrays with values
    for(uint32_t i = 0; i < N; i++) {
	    x[i] = 3;
	    y[i] = 3.3333;
    }
    
    // Number of times to execute outer loop
    int dot_times = 200;

    // Start timer
    double start = omp_get_wtime();

    float dot_prod;

    for(int j = 0; j < dot_times; j++) {
	dot_prod = 0;
        // Dot product kernel
	#pragma omp parallel for reduction(+:dot_prod)
        for(uint32_t i = 0; i < N; i++) {
            dot_prod = dot_prod + x[i] * y[i];
        }
    }

    // End timer
    double end = omp_get_wtime();
    
    // Complete the code below:

    // Number of threads used
    int num_threads = omp_get_max_threads();

    // Time taken to execute
    float time = end - start;

    // Number of floating point operations performed
    float total_ops = 2.0 * dot_times * N;
    
    // There was an integer multiply issue here, 2.0 was multiped in last, overflowing the int in
    // early multiplications
    float bytes_read = 2.0 * dot_times * N * sizeof(float);

    // Clock speed of CPU (GHz)s
    float clock_rate = 2.1;

    // Number of FP operations (FLOPs) performed per clock cycle of CPU
    float ops_per_clk = (float) total_ops / (clock_rate * 1000000000 * time) ;

    float gb_per_sec = (float) bytes_read / (time * 1024.0 * 1024.0 * 1024.0);

    float peak_gb_per_sec = 110; 

    float per_of_mem_peak = 100.0 * gb_per_sec / peak_gb_per_sec;

    // Number of FP operations (FLOPs) performed per cycle of CPU by a single thread
    float ops_per_clk_per_thread = ops_per_clk / num_threads;

    // Peak (theoretical) floating point operations per clock cycle on a single core
    // 2 FPUs/VPUs per CPU * 16 Single Per FP per AVX Instruction * 2 Fused Multiply/Add 
    float peak_ops_per_clk = 64;

    // Number of CPU cores in a CPU socket
    float num_cores = 24;

    // Number of FP operations a single socket can perform in a clock cycle
    float peak_ops_per_clk_per_skt = peak_ops_per_clk * num_cores;

    // Percentage of peak FLOPs per thread performed by a single thread
    float per_of_peak_ops_per_thd = 100.0 * ops_per_clk_per_thread / peak_ops_per_clk;

    // Percentage of peak FLOPs per socket performed overall
    float per_of_peak_ops_skt = 100.0 * ops_per_clk / peak_ops_per_clk_per_skt;

    printf("================== Results ====================\n");
    printf("%-31s: %10ld\n", "Number of elements per array" , 2 * N);
    printf("%-31s: %10.2f MB\n\n", "Size of each array" ,(float) sizeof(float) * (float) N / mb);

    printf("%-31s: %10.4e\n", "Calculated dot product", dot_prod);
    // Theoretical dot product result
    printf("%-31s: %10.4e\n\n", "Correct dot product", -1);

    printf("%-31s: %10d\n", "Times dot product calculated", dot_times);

    printf("%-31s: %10d\n", "Number of threads", num_threads);
    printf("%-31s: %10.3lf s\n\n", "Time taken" , time);

    printf("%-31s: %10.3e\n", "Total FP operations", total_ops);
    printf("%-31s: %10.1f GHz\n", "Clock rate", clock_rate);
    printf("%-31s: %10.4f\n", "FLOPs per clock performed" , ops_per_clk);
    printf("%-31s: %10.4f\n", "FLOPs per clock per thread", ops_per_clk_per_thread);
    printf("%-31s: %10.3f %%\n", "Percentage of peak thread FLOPs", per_of_peak_ops_per_thd);
    printf("%-31s: %10.3f %%\n", "Percentage of peak socket FLOPs" ,per_of_peak_ops_skt);
    printf("%-31s: %10.3f %%\n", "Percentage of peak memory", per_of_mem_peak);

}

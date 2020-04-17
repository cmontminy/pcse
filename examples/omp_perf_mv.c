
// SDS374C OMP Performance Measurement

#include <omp.h>
#include <stdint.h> // For specific-width integers
#include <stdio.h>
#include <math.h>

int main() {
    const uint32_t M = 200;
    const uint32_t N = 2*2*2*3*3*5 * pow(2,17);
    const float mb = 1024.0 * 1024.0;

    // Allocate arrays
    float * mat = malloc( (long) M*N * sizeof(float));
    float * vec = malloc(N * sizeof(float));
    float * res = malloc(M * sizeof(float));

    
    if(mat == NULL) {
        printf("Allocation of array mat failed!\n");
        exit(-1);
    }
    if(vec == NULL) {
        printf("Allocation of array vec failed!\n");
        exit(-1);
    }
    if(res == NULL) {
        printf("Allocation of array res failed!\n");
        exit(-1);
    }

    // Initialize arrays with values
    for(uint64_t i = 0; i < (long) M*N; i++) {
	    mat[i] = 3.3333;
    }
    for(uint32_t i = 0; i < N; i++) {
	    vec[i] = 3;
    }
    for(uint32_t i = 0; i < M; i++) {
	    res[i] = 0;
    }

    ///Cache use
    // Number of times to execute outer loop
    int dot_times = 1;

    //Chunk size in MB
    float C_size_MB=0.5;
    
    //Chunk size - number of elements
    int C_size= C_size_MB*1024*1024/sizeof(float);

    //Number of chunks
    int C = N/C_size;
    
    ///No cache use
    //C = 1;
    //C_size=N;

    // Start timer
    double start = omp_get_wtime();

    for(int i = 0; i < dot_times; i++) {
        
        // Dot product kernel
        for(uint32_t c = 0; c < C; c++){

#pragma omp parallel for
            for(uint64_t m = 0; m < M; m++)
            {
                for(uint32_t n=c*C_size; n<(c+1)*C_size ; n++) {
                        res[m] = res[m] + mat[m*N + n] * vec[n]; 
                        //Reading (2) elem 
                        //  - 1(res[m]) in register 
                        //      'm' does not change for 'n' loop
                        //  - (2) elems constantly
                        
                        //Writing 0 elem
                        //  - Not writing anything 

                        //No improvement compared to dotproduct
                        // 1) 
                }
            }
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
    float total_ops = 2.0 * dot_times * M * N; // 2-> mult + sum
    
    // There was an integer multiply issue here, 2.0 was multiped in last, overflowing the int in
    // early multiplications
    float bytes_read = 2.0 * dot_times * M * N * sizeof(float);

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

    printf("%-31s: %10ld\n", "Number of elements per chunk" , C_size);
    printf("%-31s: %10ld MB\n", "Size of each chunk" , C_size_MB);
    printf("%-31s: %10ld \n\n", "Number of chunks" , C);


    printf("%-31s: %10.4e\n", "Calculated result(first elem)", res[0]);
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

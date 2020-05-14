// Main code for IDEA project

#include "idea.h"

#include <stdio.h>
#include <omp.h>

void encrypt(int64_t *, int64_t *, int);

void main(int argc, char *argv[]) {

    // set number of threads
    #ifdef _OPENMP
    omp_set_num_threads(8);
    #endif
    // Dummy parallel region to startup threads
    #pragma omp parallel
    {   
        #ifdef _OPENMP
        printf("This is thread %d\n", omp_get_thread_num());
        #endif
    }

    // declare variables
    double time_start;
    double time_end;

    double malloc_time;
    double split_time;
    double keys_time;
    double encrypt_time;

    // pass [key] [infile]

    // check if param number is correct
    if (argc != 3) {
        printf("Usage: [key] [infile]\n");
        exit(1);
    }

    char *str_key = argv[1];
    char *infile = argv[2];
    
    int64_t key;
    key = convert_key(str_key);

    // check if infile exists
    if (!file_exists(infile)) {
        printf(
            "Could not find file %s. Make sure file is in current working "
            "directory.\n",
            infile);
        exit(1);
    }
    

    //check if key was passed and length is correct
    if (strlen(str_key) != 8) {
        printf(
            "Key not acceptable. Make sure your key is a 8 character long "
            "string\n");
        exit(1);
    }

    // encoding file

    // determine how many chunks there are
    FILE *fp = fopen(infile, "r");
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    // add one to num_chunks if theres leftover
    int num_chunks = (size % 8 != 0) ? (size / 8) + 1 : size / 8;

    // malloc an array based on number of chunks and for keys
    time_start = gtod_timer();
    int64_t *chunks = (int64_t*)malloc(num_chunks);
    int64_t keys[52];
    time_end = gtod_timer();
    malloc_time = time_end - time_start;

    // split file based on chunk size
    time_start = gtod_timer();
    split_file(fp, chunks, num_chunks);
    time_end = gtod_timer();
    split_time = time_end - time_start;

    // generate keys based on passed key
    time_start = gtod_timer();
    gen_sub_keys(key, keys);
    time_end = gtod_timer();
    keys_time = time_end - time_start;

    // run encryption
    time_start = gtod_timer();
    encrypt(chunks, keys, num_chunks);
    time_end = gtod_timer();
    encrypt_time = time_end - time_start;

    // write chunks out
    fp = fopen("out.txt", "w");
    fwrite(chunks, 8, num_chunks, fp);

    //free(chunks);

    // Number of threads used
    int num_threads = 1;
    #ifdef _OPENMP
    num_threads = omp_get_max_threads();
    #endif

    // Number of floating point operations performed
    float total_ops = 255 * num_chunks; // 255 ops for encrypting
    
    // There was an integer multiply issue here, 2.0 was multiped in last, overflowing the int in
    // early multiplications
    float bytes_read = size;

    // Clock speed of CPU (GHz)s
    float clock_rate = 2.1;

    // Number of FP operations (FLOPs) performed per clock cycle of CPU
    float ops_per_clk = (float) total_ops / (clock_rate * 1000000000 * encrypt_time) ;

    float gb_per_sec = (float) bytes_read / (encrypt_time * 1024.0 * 1024.0 * 1024.0);

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

    // print output
    printf("\n---------- Summary ----------\n");
    printf("%-35s: %10s\n", "File encrypted", infile);
    printf("%-35s: %10s\n", "Key", str_key);
    printf("%-35s: %10d MB\n", "Size of file", size / 1000);
    printf("%-35s: %10d\n", "Number of chunks", num_chunks);

    printf("\n---------- Timing -----------\n");
    printf("%-35s: %10.3f\n", "Malloc", malloc_time);
    printf("%-35s: %10.3f\n", "Split file", split_time);
    printf("%-35s: %10.3f\n", "Sub key generation", keys_time);
    printf("%-35s: %10.3f\n", "Encryption", encrypt_time);

    printf("\n--------- Computing ---------\n");
    printf("%-35s: %10.3e\n", "Total FP operations", total_ops);
    printf("%-35s: %10.1f GHz\n", "Clock rate", clock_rate);
    printf("%-35s: %10.4f\n", "FLOPs per clock performed" , ops_per_clk);
    printf("%-35s: %10.4f\n", "FLOPs per clock per thread", ops_per_clk_per_thread);
    printf("%-35s: %10.3f %%\n", "Percentage of peak thread FLOPs", per_of_peak_ops_per_thd);
    printf("%-35s: %10.3f %%\n", "Percentage of peak socket FLOPs" ,per_of_peak_ops_skt);
    printf("%-35s: %10.3f %%\n", "Percentage of peak memory", per_of_mem_peak);

    printf("\n---------- OpenMP -----------\n");
    #ifdef _OPENMP
    printf("%-25s: %d\n", "OMP: Number of threads", omp_get_max_threads());
    #endif

}

void encrypt(int64_t *chunks, int64_t *keys, int num_chunks) {

    int16_t steps[14];
    int16_t subchunks[4];
    int64_t temp;
    int64_t final_chunk;
    int64_t offset;

    for (int i = 0; i < num_chunks; i++) {
        // split chunk
        uint64_t mask = 0xFFFF; 
        subchunks[0] = chunks[i] & (mask << 48);
        subchunks[1] = chunks[i] & (mask << 32);
        subchunks[2] = chunks[i] & (mask << 16);
        subchunks[3] = chunks[i] & mask;

        // rounds- 224 total ops
        for (int j = 0; j < 8; j++) {
            offset = j*6;
            steps[0] = (subchunks[0] * keys[(offset)]) % 17;
            steps[1] = (subchunks[1] + keys[(offset) + 1]) % 16;
            steps[2] = (subchunks[2] + keys[(offset) + 2]) % 16;
            steps[3] = (subchunks[3] * keys[(offset) + 3]) % 17;

            steps[4] = steps[0] ^ steps[2];
            steps[5] = steps[1] ^ steps[3];

            steps[6] = (steps[4] * keys[(offset) + 4]) % 17;
            steps[7] = (steps[5] + steps[6]) % 16;
            steps[8] = (steps[7] * keys[(offset) + 5]) % 17;
            steps[9] = (steps[6] +  steps[8]) % 16;

            steps[10] = steps[0] ^ steps[8];
            steps[11] = steps[2] ^ steps[8];
            steps[12] = steps[1] ^ steps[9];
            steps[13] = steps[3] ^ steps[9];
        }

        subchunks[0] = (subchunks[0] * keys[48]) % 17;
        subchunks[1] = (subchunks[1] + keys[49]) % 16;
        subchunks[2] = (subchunks[2] + keys[50]) % 16;
        subchunks[3] = (subchunks[3] * keys[51]) % 17;

        // combine chunk back together - 16 total ops
        for (int x = 0; x < 4; x++) {
            temp = subchunks[x];
            temp = temp << (48 - (16*x));
            final_chunk |= temp;
        }

        chunks[i] = final_chunk;
    }
}
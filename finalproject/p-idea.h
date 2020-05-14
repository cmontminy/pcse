// Header File for IDEA code
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define ARRAYSIZE 52
#define KEYLEN 64
#define SUBKEYLEN 8
#define KEYSHIFT 25

// Timer (from previous assignment)
double gtod_timer() {
    
    double gtod_secbase = 0.0E0;
    struct timeval tv;
    void *Tzp=0;
    double sec;

    gettimeofday(&tv, Tzp);

    /*Always remove the LARGE sec value
    for improved accuracy  */
    if(gtod_secbase == 0.0E0) {
        gtod_secbase = (double)tv.tv_sec;
    }
        
    sec = (double)tv.tv_sec - gtod_secbase;

    return sec + 1.0E-06*(double)tv.tv_usec;
}

// Generate Subkeys
void gen_sub_keys(int64_t key, int64_t *arr) {

    int64_t mask = 0xFF;
    uint64_t temp_key;

    for (int i = 0; i < ARRAYSIZE; i += 8) {
        for (int j = 0; j <= 7; j++) {
            arr[i + j] = key & (mask << (SUBKEYLEN * j));
        }

        mask = 0xFFFFFF8;
        temp_key = (key & (mask << (KEYLEN - KEYSHIFT)));
        key = (key << KEYSHIFT) | (temp_key >> (KEYLEN - KEYSHIFT));
    }
}

// File searcher
int file_exists(char *filename) {
    struct dirent *de;
    DIR *dr = opendir(".");

    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, filename)) {
            return 1;
        }
    }
    return 0;
}

// Split File
void split_file(FILE *fp, int64_t *arr, int num_chunks) {

    for (int i = 0; i < num_chunks; i++) {
        fread(&arr[i], 1, 8, fp);
    }

    fclose(fp);
}

int64_t convert_key(char* str) {

    int64_t result;
    int64_t temp;

    result = str[0];
    for (int i = 1; i <= 7; i++) {
        temp = str[i];
        for (int j = 1; j <= i; j++) {
            temp = temp << __CHAR_BIT__;
        }
        result |= temp;
    }

    return result;
}

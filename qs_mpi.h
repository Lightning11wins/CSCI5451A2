#ifndef CLUSTER_H_
#define CLUSTER_H_
#include <math.h>
#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// OSX timer includes
#ifdef __MACH__
    #include <mach/mach.h>
    #include <mach/mach_time.h>
#endif

#define gatherer_rank 0
#define output_filename "../output.txt"

#define parse_int(str) ((int) strtol((str), (char**) NULL, 10))
#define timer_start() const double start_time = MPI_Wtime()
#define timer_stop() const double end_time = MPI_Wtime(), total_time = end_time - start_time
#define timer_print() print_time(total_time)
#define print_time(seconds) printf("Process %d: Complted after %0.04fs\n", my_rank, seconds)
#define terminate() MPI_Finalize(); return 0;

#define swap(arr, i, j) \
    unsigned int temp = arr[i];  \
    arr[i] = arr[j];    \
    arr[j] = temp;

// Integer comparison functin for using qsort().
int compare(const void *a, const void *b) {
    return (*(unsigned int *)a - *(unsigned int *)b);
}

// Helper function to find medians.
static inline unsigned int median(unsigned int* nums, int len) {
    qsort(nums, len, sizeof(unsigned int), compare);
    return (len % 2 == 1) ? nums[len / 2] : (nums[len / 2 - 1] + nums[len / 2]) / 2;
}

// Make arrays into char* for debugging.
void stringify_array(int* arr, int n, char *buffer) {
    int offset = sprintf(buffer, "[");
    for (int i = 0; i < n; i++) {
        offset += sprintf(buffer + offset, "%d, ", arr[i]);
    }
    sprintf(buffer + offset, "]");
}

// Debug function to find min and max.
static inline void findMinMax(unsigned int numbers[], size_t size, unsigned int* min, unsigned int* max) {
    if (size == 0) return;

    *min = numbers[0];
    *max = numbers[0];

    for (size_t i = 1; i < size; i++) {
        if (numbers[i] < *min) {
            *min = numbers[i];
        }
        if (numbers[i] > *max) {
            *max = numbers[i];
        }
    }
}

// Dump numbers. (For debugging)
// Was initially written by ChatGPT and revised by me, which is why it's kind of trash.
void dump(const unsigned int* numbers, char* filename, size_t size) {
    // Open the file for writing.
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    // Write each unsigned int in the numbers to the file.
    for (size_t i = 0; i < size; i++) {
        fprintf(file, "%u\n", numbers[i]);
    }

    fclose(file);
}


/**
* @brief Write an array of integers to a file.
*
* @param filename The name of the file to write to.
* @param numbers The array of numbers.
* @param num_numbers How many numbers to write.
*/
static inline void print_numbers(char const* const filename, unsigned int const* const numbers, int const num_numbers) {
    FILE * fout;

    // Open the file.
    if ((fout = fopen(filename, "w")) == NULL) {
        fprintf(stderr, "error opening '%s'\n", filename);
        abort();
    }

    // Write the header.
    fprintf(fout, "%d\n", num_numbers);

    // Write numbers to fout.
    for (int i = 0; i < num_numbers; ++i) {
        fprintf(fout, "%d\n", numbers[i]);
    }

    fclose(fout);
}

/**
* @brief Return the number of seconds since an unspecified time (e.g., Unix
*        epoch). This is accomplished with a high-resolution monotonic timer,
*        suitable for performance timing.
*
* @return The number of seconds.
*/
static inline double monotonic_seconds() {
  #ifdef __MACH__
    // OSX
    const static mach_timebase_info_data_t info;
    static double seconds_per_unit;
    if(seconds_per_unit == 0) {
        mach_timebase_info(&info);
        seconds_per_unit = (info.numer / info.denom) / 1e9;
    }
    return seconds_per_unit * mach_absolute_time();
  #else
    // Linux systems
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
  #endif
}

// Calculate Euclidean distance between two points.
static inline double euclidean_distance(const double point1[], const double point2[], int const dimensions) {
    double sum = 0.0;
    for (int i = dimensions - 1; i >= 0; i--) {
        const double diff = point1[i] - point2[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

static inline void check(const int result, const char* functionName) {
    if (result <= -1) {
        char errorBuffer[BUFSIZ];
        sprintf(errorBuffer, "cluster.c: Fail - %s", functionName);
        perror(errorBuffer);
        while (1) exit(-1);
    }
}
#endif
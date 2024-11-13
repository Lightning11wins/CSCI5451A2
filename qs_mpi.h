#ifndef CLUSTER_H_
#define CLUSTER_H_

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define gatherer_rank 0
#define output_filename "../output.txt"
#define verify_sorted
// #define output_data

#define parse_int(str) ((int) strtol((str), (char**) NULL, 10))
#define timers_init(num_timers) double start_times[num_timers] = {0}, end_times[num_timers] = {0}, total_times[num_timers] = {0};
#define timers_start(timer_id) start_times[timer_id] = MPI_Wtime()
#define timers_stop(timer_id) end_times[timer_id] = MPI_Wtime(); \
    total_times[timer_id] = end_times[timer_id] - start_times[timer_id]
#define timers_print(timer_id) printf("Process %d: Timer %d ended after %0.04fs\n", my_rank, timer_id, total_times[timer_id])
#define terminate() MPI_Finalize(); return 0;

#define swap(arr, i, j)         \
    unsigned int temp = arr[i]; \
    arr[i] = arr[j];            \
    arr[j] = temp;

// Integer comparison functin for using qsort().
int compare(const void *a, const void *b) {
    return (*(unsigned int *)a - *(unsigned int *)b);
}

/**
* @brief Write an array of integers to a file.
*
* @param filename The name of the file to write to.
* @param numbers The array of numbers.
* @param num_numbers How many numbers to write.
*/
static inline void write_numbers(char const* const filename, unsigned int const* const numbers, int const num_numbers) {
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

#endif

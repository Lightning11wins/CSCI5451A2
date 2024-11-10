#ifndef CLUSTER_H_
#define CLUSTER_H_
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// OSX timer includes
#ifdef __MACH__
    #include <mach/mach.h>
    #include <mach/mach_time.h>
#endif

#define parse_int(str) ((int) strtol((str), (char**) NULL, 10))
#define start_timer() const double start_time = MPI_Wtime()
#define stop_timer() const double stop_time = MPI_Wtime()
#define duration(start_time, stop_time) ((stop_time) - (start_time))
#define print_timer() print_time(duration(start_time, stop_time))

#define begin() MPI_Init(&argc, &argv); start_timer();
#define terminate() MPI_Finalize(); return 0;
#define end() stop_timer(); print_timer(); terminate();

// Integer comparison functin for using qsort().
int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

// Helper function to find medians.
static inline int median(int nums[], int len) {
    qsort(nums, len, sizeof(int), compare);
    return (len % 2 == 1) ? nums[len / 2] : (nums[len / 2 - 1] + nums[len / 2]) / 2;
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

/**
* @brief Output the seconds elapsed while clustering.
*
* @param seconds Seconds spent on the algorithm.
*/
static inline void print_time(double const seconds) {
    printf("Process complted after %0.04fs\n", seconds);
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
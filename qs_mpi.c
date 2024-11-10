#include "quicksort.h"

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    // Get values from MPI.
    int num_processors, my_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_processors); // Get number of processes.
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // Get rank of this process.

    // Check for argument issues.
    if (argc <= 1) {
        printf("Usage: ./quicksort <N>\n");
        terminate();
    }
    
    // Parse arguments.
    int num_total_numbers = parse_int(argv[1]);
    int num_my_numbers = num_total_numbers / num_processors;

    if (num_total_numbers % num_processors != 0) {
        printf("Warning: N (%d) is not devisible by the number of processors (%d).\n", num_total_numbers, num_processors);
        fflush(stdout);
    }

    // Generate random numbers to sort.
    srand(my_rank);
    int numbers[N] = {0};
    for (int i = 0; i < num_my_numbers; i++) {
        numbers[i] = rand();
    }
    int my_index = rand() % num_my_numbers, my_pivot = numbers[my_index];
    printf("Process %d: Picked pivot numbers[%d] = %d.\n", my_rank, my_index, my_pivot);

    // Gather all pivots.
    int pivots[num_processors] = {0};
    MPI_Allgather(&my_pivot, 1, MPI_INT, pivots, 1, MPI_INT, MPI_COMM_WORLD);

    // Pick the pivot.
    int pivot = median(pivots, num_processors);
    printf("Process %d: Pivot is %d.\n", my_rank, pivot);

    // Partition

    end();
}

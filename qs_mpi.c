#include "qs_mpi.h"

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    // Get a fresh comunicator for our process.
    MPI_Comm comm;
    MPI_Comm_dup(MPI_COMM_WORLD, &comm);

    // Get values from MPI.
    int num_processors, my_rank;
    MPI_Comm_size(comm, &num_processors); // Get number of processes.
    MPI_Comm_rank(comm, &my_rank); // Get rank of this process.

    // Check for argument issues.
    if (argc <= 1) {
        printf("Usage: ./quicksort <N>\n");
        terminate();
    }
    
    // Parse arguments.
    int num_total_numbers = parse_int(argv[1]);
    int num_my_numbers = num_total_numbers / num_processors;

    if (num_total_numbers % num_processors != 0) {
        fprintf(stderr, "Warning: N (%d) is not devisible by the number of processors (%d).\n", num_total_numbers, num_processors);
    }

    // Generate random numbers to sort.
    srand(my_rank + 1);
    int* my_numbers = malloc(num_my_numbers * sizeof(int));
    for (int i = 0; i < num_my_numbers; i++) {
        my_numbers[i] = rand();
    }

    // Begin timing the algorithm.
    start_timer();

    // Begin partitioning in parallel.
    while (num_processors > 1) {
        // Select a random pivot.
        int my_index = rand() % num_my_numbers, my_pivot = my_numbers[my_index];
        printf("Process %d: Picked pivot my_numbers[%d] = %d.\n", my_rank, my_index, my_pivot);

        // Gather all pivots in the data.
        int pivots[num_processors];
        MPI_Allgather(&my_pivot, 1, MPI_INT, pivots, 1, MPI_INT, comm);

        // Pick the pivot.
        int pivot = median(pivots, num_processors);
        printf("Process %d: Pivot is %d.\n", my_rank, pivot);

        // Partition data around the pivot.
        // Contract: Numbers before i are smaller than the pivot.
        //           Numbers before j (but not i) are larger than or equal to the pivot.
        //           After partitioning, we swap the pivot to the center.
        // Thus, we have [0..i) for the smaller partition and [i..num_my_numbers) for the larger partition.
        int i = 0; // Smaller section contains no elements yet.
        for (int j = 0; j < num_my_numbers; j++) {
            if (my_numbers[j] < pivot) {
                swap(my_numbers, i, j);
                i++;
            }
        }

        // Determine how many my_numbers will be sent where.
        int half_num_processors = num_processors / 2,
            small_numbers_per_processor = i / half_num_processors,
            large_numbers_per_processor = (num_my_numbers - i) / half_num_processors,
            data_sent_per_processor[num_processors], p = 0,
            data_sent_displacements[num_processors], displacement = 0;
        for (; p < half_num_processors; p++) {
            data_sent_per_processor[p] = small_numbers_per_processor;
            data_sent_displacements[p] = displacement;
            displacement += small_numbers_per_processor;
        }
        for (; p < num_processors; p++) {
            data_sent_per_processor[p] = large_numbers_per_processor;
            data_sent_displacements[p] = displacement;
            displacement += large_numbers_per_processor;
        }

        // Logging
        char buff[1024] = {0};
        stringify_array(data_sent_per_processor, num_processors, buff);
        printf("Process %d: Sending data to others: %s.\n", my_rank, buff);

        // Comunicate the amount of data each processor will send.
        int data_recv_per_processor[num_processors];
        MPI_Alltoall(data_sent_per_processor, 1, MPI_INT, data_recv_per_processor, 1, MPI_INT, comm);

        // Calculate displacements.
        int data_recv_displacements[num_processors];
        for (displacement = p = 0; p < num_processors; p++) {
            data_recv_displacements[p] = displacement;
            displacement += data_recv_per_processor[p];
        }

        // Displacement is the amount of data we will recv.
        num_my_numbers = displacement;

        // Transfer the data.
        // Note: data_sent_per_processor is the data this processor sends to each other processor.
        // Note: data_sent_per_processor is the data this processor recieves from each other processor.
        // Note: The fact that I have to make these notes means I need to use better variable names in the furture.
        int* new_numbers = malloc(num_my_numbers * sizeof(int));
        MPI_Alltoallv(
            my_numbers, data_sent_per_processor, data_sent_displacements, MPI_INT,
            new_numbers, data_recv_per_processor, data_recv_displacements, MPI_INT,
            comm
        );

        // Continue partitioning with the new numbers.
        free(my_numbers);
        my_numbers = new_numbers;

        // Note: We assume that, since we're partitioning a large amount of random data, the smaller and larger portions
        // will be roughly equal. Synchronizing across all the processors to determine an optimal distribution is expensive
        // and will probably create a 50%/50% split (or something close to it) in most situations, so we simply assume 50%/50%.
        // This could lead to load imbalance in rare situations, but it should be faster in the average-case senario.

        // Divide the processors in half.
        // color = 0: smaller partition, color = 1: larger partition
        int color = (my_rank < num_processors / 2) ? 0 : 1;
        MPI_Comm_split(comm, color, my_rank, &comm);

        // Update the size of this processor group.
        MPI_Comm_size(comm, &num_processors);
    }

    // Sort the data.
    qsort(my_numbers, num_my_numbers, sizeof(int), compare);
    printf("Finished sorting %d numbers.\n", num_my_numbers);

    // Refresh the world communicator to include everyone again.
    MPI_Comm_dup(MPI_COMM_WORLD, &comm);

    // Determine how many my_numbers to gather.
    int num_recv_numbers[num_processors];
    MPI_Gather(&num_my_numbers, 1, MPI_INT, num_recv_numbers, 1, MPI_INT, gatherer_rank, comm);

    // Make space for the sorted data.
    int num_actual_numbers = 0;
    if (my_rank == gatherer_rank) {
        for (int i = 0; i < num_processors; i++) {
            num_actual_numbers += num_recv_numbers[i];
            printf("Gathering %d sorted numbers from %d.\n", num_recv_numbers[i], i);
        }
        printf("Total numbers to gather: %d.\n", num_actual_numbers);
    }
    int sorted_numbers[num_actual_numbers];
    memset(sorted_numbers, 0, num_my_numbers * sizeof(int));

    // Gather the data.
    MPI_Gather(
        my_numbers, num_my_numbers, MPI_INT,
        sorted_numbers, num_actual_numbers, MPI_INT,
        gatherer_rank, MPI_COMM_WORLD
    );
    
    // Free each process's my_numbers.
    free(my_numbers);
    
    // Output the data.
    if (my_rank == gatherer_rank) {
        printf("Writing %d sorted numbers to disk.\n", num_actual_numbers);
        print_numbers("output.txt", sorted_numbers, num_total_numbers);
    }

    // Stop and print the timer, then end and clean up the program.
    end();
}

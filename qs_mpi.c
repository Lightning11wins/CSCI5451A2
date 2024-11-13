#include "qs_mpi.h"
#include "median.h"

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    timers_init(3);

    // Get a fresh comunicator for our process.
    MPI_Comm comm;
    MPI_Comm_dup(MPI_COMM_WORLD, &comm);
    MPI_Comm comm_all = comm;

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
    int target_total_numbers = parse_int(argv[1]);
    int num_my_numbers = target_total_numbers / num_processors;

    if (target_total_numbers % num_processors != 0) {
        fprintf(stderr, "Warning: N (%d) is not devisible by the number of processors (%d).\n", target_total_numbers, num_processors);
    }

    // Generate random numbers to sort.
    // Note: It appears that rank 0 and rank 1 may give identical random numbers.
    // Adding one to the rank fixed this, so I just left it at that, although I'm
    // not entirely why I had this issue in the first place.
    srand(my_rank + 1);
    unsigned int* my_numbers = (unsigned int*) malloc(num_my_numbers * sizeof(unsigned int));
    for (int i = 0; i < num_my_numbers; i++) {
        my_numbers[i] = rand();
    }

    // Begin timing the algorithm.
    timers_start(0);

    // Begin partitioning in parallel.
    while (num_processors > 1) {
        // Select a random pivot.
        unsigned int my_pivot = fast_median(my_numbers, num_my_numbers);

        // Gather all pivots in the data.
        unsigned int pivots[num_processors];
        MPI_Allgather(&my_pivot, 1, MPI_UNSIGNED, pivots, 1, MPI_UNSIGNED, comm);

        // Pick the pivot.
        unsigned int pivot = median(pivots, num_processors);

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
        int num_small_numbers = i, num_large_numbers = num_my_numbers - num_small_numbers,
            half_num_processors = num_processors / 2,
            small_numbers_per_processor = num_small_numbers / half_num_processors,
            small_numbers_remaining = num_small_numbers % half_num_processors,
            large_numbers_per_processor = num_large_numbers / half_num_processors,
            large_numbers_remaining = num_large_numbers % half_num_processors,
            data_sent_per_processor[num_processors], p = 0,
            data_sent_displacements[num_processors], displacement = 0;
        for (; p < small_numbers_remaining; p++) {
            data_sent_per_processor[p] = small_numbers_per_processor + 1;
            data_sent_displacements[p] = displacement;
            displacement += small_numbers_per_processor + 1;
        }
        for (; p < half_num_processors; p++) {
            data_sent_per_processor[p] = small_numbers_per_processor;
            data_sent_displacements[p] = displacement;
            displacement += small_numbers_per_processor;
        }
        for (; p < half_num_processors + large_numbers_remaining; p++) {
            data_sent_per_processor[p] = large_numbers_per_processor + 1;
            data_sent_displacements[p] = displacement;
            displacement += large_numbers_per_processor + 1;
        }
        for (; p < num_processors; p++) {
            data_sent_per_processor[p] = large_numbers_per_processor;
            data_sent_displacements[p] = displacement;
            displacement += large_numbers_per_processor;
        }

        // Comunicate the amount of data each processor will send.
        int data_recv_per_processor[num_processors];
        MPI_Alltoall(data_sent_per_processor, 1, MPI_UNSIGNED, data_recv_per_processor, 1, MPI_UNSIGNED, comm);

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
        unsigned int* new_numbers = (unsigned int*) malloc(num_my_numbers * sizeof(unsigned int));
        MPI_Alltoallv(
            my_numbers, data_sent_per_processor, data_sent_displacements, MPI_UNSIGNED,
            new_numbers, data_recv_per_processor, data_recv_displacements, MPI_UNSIGNED,
            comm
        );

        // Continue partitioning with the new numbers.
        free(my_numbers);
        my_numbers = new_numbers;

        // Note: We assume that, since we're partitioning a large amount of random data, the smaller and larger portions
        // will be roughly equal. Synchronizing across all the processors to determine an optimal distribution is expensive
        // and will probably create a 50%/50% split (or something close to it) in most situations, so we simply assume 50%/50%.
        // This could lead to load imbalance in rare situations, but it should be faster in the average-case senario.

        // Calculate each processor's color to divide them in half.
        int my_current_rank;
        MPI_Comm_rank(comm, &my_current_rank);
        int color = my_current_rank < num_processors / 2;

        // Spit the comunicator.
        MPI_Comm new_comm;
        MPI_Comm_split(comm, color, my_current_rank, &new_comm);
        comm = new_comm;

        // Update the size of this processor group.
        MPI_Comm_size(comm, &num_processors);
    }

    // Sort the data.
    timers_start(1);
    qsort(my_numbers, num_my_numbers, sizeof(unsigned int), compare);
    timers_stop(1);
    timers_print(1);

    // Refresh the world communicator to include everyone again.
    comm = comm_all;
    MPI_Comm_size(comm, &num_processors);

    // Determine how many my_numbers to gather.
    int num_recv_numbers[num_processors];
    memset(num_recv_numbers, 0, num_processors * sizeof(int));
    MPI_Gather(&num_my_numbers, 1, MPI_INT, num_recv_numbers, 1, MPI_INT, gatherer_rank, comm);

    // Calculate space for the sorted data.
    int num_actual_numbers = 0;
    for (int i = 0; i < num_processors; i++) {
        num_actual_numbers += num_recv_numbers[i];
    }

    // Allocate space for the sorted.
    unsigned int* all_numbers = (unsigned int*) calloc(num_actual_numbers, sizeof(unsigned int));
    if (all_numbers == NULL) {
        perror("Memory allocation failed");
        terminate();
    }

    // Calculate displacements.
    int recv_displacements[num_processors];
    for (int displacement = 0, p = 0; p < num_processors; p++) {
        recv_displacements[p] = displacement;
        displacement += num_recv_numbers[p];
    }

    // Gather the data.
    MPI_Gatherv(
        my_numbers, num_my_numbers, MPI_UNSIGNED,
        all_numbers, num_recv_numbers, recv_displacements, MPI_UNSIGNED,
        gatherer_rank, comm
    );

    // Algorithm is done, that's time!
    timers_stop(0);
    timers_print(0);

    // Free each process's my_numbers.
    free(my_numbers);

    if (my_rank == gatherer_rank) {
        // Check that data is sorted.
        #ifdef verify_sorted
        int is_sorted = 1;
        for (int i = 1; i < num_actual_numbers; i++) {
            unsigned int previous = all_numbers[i - 1], current = all_numbers[i];
            if (current < previous) {
                printf("Fail! %d >= %d is false (%s:%d)\n", current, previous, output_filename, i + 1);
                is_sorted = 0;
            }
            previous = current;
        }
        if (is_sorted == 1) {
            printf("Success! Output data is sorted in ascending order.\n");
        }
        #endif
        
        // Output the data.
        #ifdef output_data
        timers_start(2);
        write_numbers(output_filename, all_numbers, num_actual_numbers);
        timers_stop(2);
        timers_print(2);
        #endif
    }

    // Stop and print the timer, then end and clean up the program.
    terminate();
}

# Assignment #2: Quicksort
## Algorithm Parallelization
The bulk of the difficult logic for quicksorting in parallel comes from dividing the data in parallel. We need to produce `p` chunks of data such that, for any chunk C<sub>i</sub>, max(C<sub>i</sub>) < min(C<sub>i+1</sub>) + 1.

In order to produce these chunks *in parallel*, we first agree on a pivot and then use it to partition the data into a smaller and larger chunk *in parallel* on each processor. We agree on the pivot by having each processor select a random piece of data, then they comunicate that data to every other processor (`MPI_Allgather()`) so they can each calculate the median (which will be the same for every processor in the communicator). Once each processor has its small and large pieces, it divides the small piece into chunks that it will send to half of the processors, and the large piece into chunks that it will send to the other half of the processors. The processors comunicate how many chunks each is expecting to recieve (`MPI_Alltoall()`), then they transmit and recieve the chunks (`MPI_Alltoallv()`). Now, half the processors have data that is smaller than the data held by the other half. We split the comunicator along these halves (`MPI_Comm_split()`) and each half recursively repeats the above process. This recursive parallel partitioning continues until every processor is the only one in its comunicator.

Once the data is partitioned, we simply call `qsort()` to sort the data. Then, the processors send their sorted data (`MPI_Gatherv()`) to a gathering processor (specified in `qs_mpi.h1` as `gatherer_rank`, which is `0` by default) which is in charge of collecting and writing the data to disk.

### Median Calculation
To calculate the median, this program uses a helper function from `median.h` called `fast_median()`. This basically works by taking the median of only the first `8192` numbers in the data. We assume this is a representative sample because the data is randomly distributed. Note that this would cause issues if the data were *not* randomly distributed. However, taking the median of the entire set of data only requires about 20% longer. Since `fast_median()` still needs to calculate a median, we use a linear-time algorithm from geeksforgeeks.org, which is cited in `medians.h`. Prior to this implementation, `fast_median()` used `slow_median()`, which is a nieve median implementation that sorts the data and takes the middle element. As it turns out, this only slowed down the program by around 4-5%, likely because the size of the input data was only `8192`. Using `slow_median()` directly, instead of inside `fast_median()` slows down the program by closer to 120%.

**tl;dr** This implementation partitions using `fast_median()` which calls `median()` (linear time) on a representative sample of the data.

## Timing
I measured the process which took the longest time to complete sorting *and* partitioning of the data. The load balancing was decent. Improving the sampling size for medians to determine the pivot made it significantly better, however, that also slowed down median calculation. I picked a sample size of `8192` numbers, as mentioned above, because it seemed to give the best results.

### Time in Seconds
| Size | 1 Processor | 2 Processors | 4 Processors | 8 Processors | 16 Processors |
|------|-------------|--------------|--------------|--------------|---------------|
| 1M   | 0.6194s     | 0.3161s      | 0.1906s      | 0.1538s      | 0.1001s       |
| 10M  | 7.2316s     | 3.6825s      | 1.8815s      | 0.9511s      | 0.7116s       |
| 100M | 81.8201s    | 42.9335s     | 21.3860s     | 10.9921s     | 5.7401s       |

### Speedup Compared to 1 Processor
| Size | 1 Processor | 2 Processors | 4 Processors | 8 Processors | 16 Processors |
|------|-------------|--------------|--------------|--------------|---------------|
| 1M   | 1.0000x     | 1.9595x      | 3.2497x      | 4.0273x      | 6.1878x       |
| 10M  | 1.0000x     | 1.9638x      | 3.8435x      | 7.6034x      | 10.1625x      |
| 100M | 1.0000x     | 1.9057x      | 3.8259x      | 7.4435x      | 14.2541x      |

As you can see, the efficency of this code really starts to drop off with larger numbers of processors, unless the size of the data is sufficent enough to overcome the overhead. This may be due to the median sampling, and it's possible that this phenominion would play out differently with a different median sampling size. Either way, I learned a lot from analizing these results.

Note: Reach out to me if you need further analysis or clarification of these results.

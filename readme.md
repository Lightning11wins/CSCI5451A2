# Assignment #2: Quicksort
## Algorithm Parallelization
The bulk of the difficult logic for quicksorting in parallel comes from dividing the data in parallel. We need to produce `p` chunks of data such that, for any chunk C<sub>i</sub>, max(C<sub>i</sub>) < min(C<sub>i+1</sub>) + 1.

In order to produce these chunks *in parallel*, we first agree on a pivot and then use it to partition the data into a smaller and larger chunk *in parallel* on each processor. We agree on the pivot by having each processor select a random piece of data, then they comunicate that data to every other processor (`MPI_Allgather()`) so they can each calculate the median (which will be the same for every processor in the communicator). Once each processor has its small and large pieces, it divides the small piece into chunks that it will send to half of the processors, and the large piece into chunks that it will send to the other half of the processors. The processors comunicate how many chunks each is expecting to recieve (`MPI_Alltoall()`), then they transmit and recieve the chunks (`MPI_Alltoallv()`). Now, half the processors have data that is smaller than the data held by the other half. We split the comunicator along these halves (`MPI_Comm_split()`) and each half recursively repeats the above process. This recursive parallel partitioning continues until every processor is the only one in its comunicator.

Once the data is partitioned, we simply call `qsort()` to sort the data. Then, the processors send their sorted data (`MPI_Gatherv()`) to a gathering processor (specified in `qs_mpi.h1` as `gatherer_rank`, which is `0` by default) which is in charge of collecting and writing the data to disk.

### Median Calculation
To calculate the median, this program uses a helper function from `median.h` called `fast_median()`. This basically works by taking the median of only the first `8192` numbers in the data. We assume this is a representative sample because the data is randomly distributed. Note that this would cause issues if the data were *not* randomly distributed. However, taking the median of the entire set of data only requires about 20% longer. Since `fast_median()` still needs to calculate a median, we use a linear-time algorithm from geeksforgeeks.org, which is cited in `medians.h`. Prior to this implementation, `fast_median()` used `slow_median()`, which is a nieve median implementation that sorts the data and takes the middle element. As it turns out, this only slowed down the program by around 4-5%, likely because the size of the input data was only `8192`. Using `slow_median()` directly, instead of inside `fast_median()` slows down the program by closer to 120%.

**tl;dr** This implementation partitions using `fast_median()` which calls `median()` (linear time) on a representative sample of the data.

## Timing
### OpenMP
| Clusters  | 1 Thread   | 2 Threads | 4 Threads | 8 Threads | 16 Threads |
|-----------|------------|-----------|-----------|-----------|------------|
| 256       | 1373.5732s | 686.3298s | 345.2970s | 172.2829s | 87.0074s   |
| 512       | 370.8213s  | 190.0222s | 95.6559s  | 48.9559s  | 23.8680s   |
| 1024      | 339.7752s  | 178.3820s | 88.7130s  | 44.3847s  | 22.7710s   |

### pthreads
| Clusters  | 1 Thread  | 2 Threads | 4 Threads | 8 Threads | 16 Threads |
|-----------|-----------|-----------|-----------|-----------|------------|
| 256       | Too long* | 684.3861s | 351.9231s | 188.6727s | 98.4924s   |
| 512       | 374.6625s | 191.3310s | 97.6285s  | 49.0085s  | 25.2256s   |
| 1024      | 342.8823s | 171.9045s | 89.2886s  | 45.1169s  | 22.8688s   |

*This took too long to be properly tested. I can make the educated guess that it'd take around 1373.5732s, though.

Note: Reach out to me if you'd like more analysis or clarification of these results
#ifndef MEDIAN_H_
#define MEDIAN_H_

/**
 * The code for median() was copied from an implementation by geeksforgeeks.org, which was written in C++.
 * I modified their original code to create this version in C. The code for slow_median() and fast_median()
 * was entirely written by me, as was all other code in this project.
 * 
 * Original Source: https://geeksforgeeks.org/median-of-an-unsorted-array-in-liner-time-on
 */

// Utility function to swapping of element.
static inline void swap_ptrs(unsigned int* a, unsigned int* b) {
    unsigned int temp = *a;
    *a = *b;
    *b = temp;
}

static inline int partition(unsigned int arr[], int l, int r) {
    unsigned int lst = arr[r];
    int i = l, j = l;
    while (j < r) {
        if (arr[j] < lst) {
            swap_ptrs(&arr[i], &arr[j]);
            i++;
        }
        j++;
    }
    swap_ptrs(&arr[i], &arr[r]);
    return i;
}
 
// Picks a random pivot element between
// l and r and partitions arr[l..r]
// around the randomly picked element
// using partition()
static inline unsigned int randomPartition(unsigned int arr[], int l, int r) {
    int n = r - l + 1;
    unsigned int pivot = rand() % n;
    swap_ptrs(&arr[l + pivot], &arr[r]);
    return partition(arr, l, r);
}
 
// Utility function to find median
void medianUtil(unsigned int arr[], int l, int r, int k, unsigned int* a, unsigned int* b) {
    if (l <= r) {
        // Find the partition index
        unsigned int partitionIndex = randomPartition(arr, l, r);
 
        // If partition index = k, then
        // we found the median of odd
        // number element in arr[]
        if (partitionIndex == k) {
            *b = arr[partitionIndex];
            if (*a != -1) return;
        }
 
        // If index = k - 1, then we get
        // a & b as middle element of
        // arr[]
        else if (partitionIndex == k - 1) {
            *a = arr[partitionIndex];
            if (*b != -1) return;
        }
 
        // If partitionIndex >= k then
        // find the index in first half
        // of the arr[]
        if (partitionIndex >= k) return medianUtil(arr, l, partitionIndex - 1, k, a, b);

        // If partitionIndex <= k then
        // find the index in second half
        // of the arr[]
        else return medianUtil(arr, partitionIndex + 1, r, k, a, b);
    }
}
 
// Function to find Median
static inline int median(unsigned int arr[], int n) {
    unsigned int ans, a = -1, b = -1;
 
    // If n is odd
    if (n % 2 == 1) {
        medianUtil(arr, 0, n - 1, n / 2, &a, &b);
        ans = b;
    }
 
    // If n is even
    else {
        medianUtil(arr, 0, n - 1, n / 2, &a, &b);
        ans = (a + b) / 2;
    }

    return ans;
}

// Simple aproach that sorts the data. It's rather slow,
// but it turned out that using it for fast_median instead
// of the implementation above was only slower by about 4-5%.
static inline unsigned int slow_median(unsigned int* nums, int len) {
    qsort(nums, len, sizeof(unsigned int), compare);
    return (len % 2 == 1) ? nums[len / 2] : (nums[len / 2 - 1] + nums[len / 2]) / 2;
}

// Guess the median by looking at a few numbers. Assumes nums >= 32768.
static inline unsigned int fast_median(unsigned int* nums, int len) {
    return median(nums, 8192);
}

#endif

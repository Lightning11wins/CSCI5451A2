#include <stdio.h>
#include <stdlib.h>

#define output_filename "output.txt"

int main() {
    FILE *file = fopen(output_filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    int lines;
    if (fscanf(file, "%d", &lines) != 1) {
        fprintf(stderr, "Error reading number of lines.\n");
        fclose(file);
        return 1;
    }

    int previous, current;
    if (fscanf(file, "%d", &previous) != 1) {
        fprintf(stderr, "Error reading first number.\n");
        fclose(file);
        return 1;
    }

    int is_sorted = 1; // Flag to keep track of sorted order
    for (int i = 1; i < lines; i++) {
        if (fscanf(file, "%d", &current) != 1) {
            fprintf(stderr, "Error reading number at line %d.\n", i + 1);
            return 1;
        }
        if (current < previous) {
            printf("Fail! %d < %d is false (%s:%d)\n", current, previous, output_filename, i + 1);
            is_sorted = 0;
            break;
        }
        previous = current;
    }

    fclose(file);

    if (is_sorted) {
        printf("SUCCESS - The numbers are sorted in non-decreasing order.\n");
    } else {
        printf("FAIL - The numbers are NOT sorted in non-decreasing order.\n");
    }

    return 0;
}

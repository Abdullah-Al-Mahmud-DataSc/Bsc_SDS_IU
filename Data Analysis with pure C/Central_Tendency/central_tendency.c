#include <stdio.h>
#include <stdlib.h>   // atof
#include <string.h>   // strcmp
#include <math.h>     // log, exp (needed for geometric mean)

#define MAX_ROWS 50000

// Copy column number `col` of a CSV line into `out`.
// Returns 1 if the column exists, 0 otherwise.
int get_field(char line[], int col, char out[]) {
    int current = 0;
    int j = 0;
    for (int i = 0; ; i++) {
        char c = line[i];
        if (c == ',' || c == '\n' || c == '\r' || c == '\0') {
            if (current == col) {
                out[j] = '\0';
                return 1;
            }
            if (c != ',') return 0;   // end of line, column not found
            current++;
        } else if (current == col) {
            out[j] = c;
            j++;
        }
    }
}

int main() {
    char filename[] = "../sales_dataset.csv";
    char target_col[] = "Sales";

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Cannot open file %s\n", filename);
        return 1;
    }

    // ---- find the target column in the header ----
    char line[1024];
    char field[256];
    fgets(line, sizeof(line), fp);

    int target_index = -1;
    for (int col = 0; get_field(line, col, field); col++) {
        if (strcmp(field, target_col) == 0) {
            target_index = col;
            break;
        }
    }
    if (target_index == -1) {
        printf("Error: column %s not found\n", target_col);
        fclose(fp);
        return 1;
    }

    // ---- read the data in a single pass ----
    double data[MAX_ROWS];
    int n = 0;
    while (n < MAX_ROWS && fgets(line, sizeof(line), fp) != NULL) {
        if (get_field(line, target_index, field)) {
            data[n] = atof(field);
            n++;
        }
    }
    fclose(fp);

    if (n == 0) {
        printf("No data found\n");
        return 1;
    }

    printf("File:   %s\n", filename);
    printf("Column: %s\n", target_col);
    printf("Rows:   %d\n", n);
    printf("-------------------------------------------\n");

    // ---- sort (insertion sort) ----
    for (int i = 1; i < n; i++) {
        double key = data[i];
        int j = i - 1;
        while (j >= 0 && data[j] > key) {
            data[j + 1] = data[j];
            j--;
        }
        data[j + 1] = key;
    }

    // ---- arithmetic, geometric and harmonic means ----
    double sum = 0, log_sum = 0, recip_sum = 0;
    int positives = 0;
    for (int i = 0; i < n; i++) {
        sum += data[i];
        if (data[i] > 0) {
            log_sum += log(data[i]);
            recip_sum += 1.0 / data[i];
            positives++;
        }
    }

    printf("Arithmetic Mean : %.4f\n", sum / n);
    if (positives > 0) {
        printf("Geometric Mean  : %.4f\n", exp(log_sum / positives));
        printf("Harmonic Mean   : %.4f\n", positives / recip_sum);
    } else {
        printf("Geometric Mean  : N/A (no positive values)\n");
        printf("Harmonic Mean   : N/A (no positive values)\n");
    }

    // ---- median ----
    double median;
    if (n % 2 == 1)
        median = data[n / 2];
    else
        median = (data[n / 2 - 1] + data[n / 2]) / 2.0;
    printf("Median          : %.4f\n", median);

    // ---- mode (data is sorted, so equal values sit together) ----
    double mode = data[0];
    int max_count = 1;
    int run = 1;
    for (int i = 1; i < n; i++) {
        if (data[i] == data[i - 1])
            run++;
        else
            run = 1;

        if (run > max_count) {
            max_count = run;
            mode = data[i];
        }
    }

    if (max_count == 1)
        printf("Mode            : No mode (all values are unique)\n");
    else
        printf("Mode            : %.4f (appears %d times)\n", mode, max_count);

    return 0;
}
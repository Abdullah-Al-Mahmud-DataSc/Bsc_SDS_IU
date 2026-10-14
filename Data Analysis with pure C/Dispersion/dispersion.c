#include <stdio.h>
#include <stdlib.h>   // atof
#include <string.h>   // strcmp
#include <math.h>     // sqrt (needed for standard deviation)

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

// Quartile by position: multiplier 1 = Q1, 2 = Q2, 3 = Q3 (data must be sorted)
double quartile(double data[], int n, int multiplier) {
    double pos = multiplier * (n + 1) / 4.0;
    int whole = (int)pos;
    double frac = pos - whole;

    if (whole < 1) return data[0];
    if (whole >= n) return data[n - 1];
    return data[whole - 1] + frac * (data[whole] - data[whole - 1]);
}

double absolute(double x) {
    if (x < 0) return -x;
    return x;
}

int main() {
    char filename[] = "../Customer_data.csv";
    char target_col[] = "UnitPrice";

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error: could not open %s\n", filename);
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
        printf("Error: column \"%s\" not found in header\n", target_col);
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

    printf("File:      %s\n", filename);
    printf("Column:    %s\n", target_col);
    printf("Total data rows found: %d\n", n);
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

    printf("First 10 sorted values: ");
    for (int i = 0; i < 10 && i < n; i++) printf("%.4f ", data[i]);
    printf("\n");
    printf("-------------------------------------------\n");

    // ---- max, min, range ----
    double max = data[n - 1];
    double min = data[0];
    double range = max - min;
    printf("The maximum value is : %.4f\n", max);
    printf("The minimum value is : %.4f\n", min);
    printf("The range is : %.4f\n", range);
    printf("-------------------------------------------\n");

    // ---- mean ----
    double sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    double mean = sum / n;
    printf("Mean is %.4f\n", mean);

    // ---- variance, standard deviation, mean deviation (one loop) ----
    double sq_sum = 0;
    double abs_sum = 0;
    for (int i = 0; i < n; i++) {
        double diff = data[i] - mean;
        sq_sum += diff * diff;
        abs_sum += absolute(diff);
    }
    double variance = sq_sum / n;
    double std = sqrt(variance);
    double mean_dev = abs_sum / n;
    printf("The variance is %.4f\n", variance);
    printf("The standard deviation is: %.4f\n", std);
    printf("The mean deviation is : %.4f\n", mean_dev);

    // ---- quartiles ----
    double q1 = quartile(data, n, 1);
    double q2 = quartile(data, n, 2);
    double q3 = quartile(data, n, 3);
    printf("Q1 is: %.4f\n", q1);
    printf("Q2 or Median is: %.4f\n", q2);
    printf("Q3 is: %.4f\n", q3);

    double iqr = q3 - q1;
    printf("The quartile deviation is: %.4f\n", iqr / 2.0);
    printf("The inter quartile range (IQR) is: %.4f\n", iqr);
    printf("-------------------------------------------\n");

    // ---- relative (coefficient) measures ----
    if (max + min != 0)
        printf("Coefficient of range is: %.4f\n", range / (max + min));
    else
        printf("Coefficient of range is: undefined (max + min = 0)\n");

    if (mean != 0) {
        printf("Coefficient of variation is: %.4f %%\n", std / mean * 100);
        printf("Coefficient of mean deviation about mean is: %.4f\n", mean_dev / mean);
    } else {
        printf("Coefficient of variation: undefined (mean = 0)\n");
        printf("Coefficient of mean deviation: undefined (mean = 0)\n");
    }

    if (q3 + q1 != 0)
        printf("The coefficient of quartile deviation is: %.4f\n", iqr / (q3 + q1));
    else
        printf("The coefficient of quartile deviation is: undefined (Q3 + Q1 = 0)\n");

    return 0;
}
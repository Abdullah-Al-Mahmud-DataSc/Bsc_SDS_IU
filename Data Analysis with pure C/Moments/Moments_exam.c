#include <stdio.h>
#include <stdlib.h>   // atof
#include <string.h>   // strcmp
#include <math.h>     // sqrt (needed for skewness)

#define MAX_ROWS 10000

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

double absolute(double x) {
    if (x < 0) return -x;
    return x;
}

int main() {
    char path[] = "../StudentPerformanceFactors.csv";
    char target_col[] = "Attendance";

    /* STEP 1: open the file */
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("Error: could not open the file '%s'\n", path);
        return 1;
    }

    /* STEP 2: find the column in the header, then read its values */
    char line[512];
    char field[128];
    fgets(line, sizeof(line), file);

    int target_index = -1;
    for (int col = 0; get_field(line, col, field); col++) {
        if (strcmp(field, target_col) == 0) {
            target_index = col;
            break;
        }
    }
    if (target_index == -1) {
        printf("Error: column '%s' not found\n", target_col);
        fclose(file);
        return 1;
    }

    double x[MAX_ROWS];
    int n = 0;
    while (n < MAX_ROWS && fgets(line, sizeof(line), file) != NULL) {
        if (get_field(line, target_index, field)) {
            x[n] = atof(field);
            n++;
        }
    }
    fclose(file);

    if (n == 0) {
        printf("No data found\n");
        return 1;
    }
    printf("Loaded %d values from the %s column.\n\n", n, target_col);

    /* STEP 3: raw moments about the origin */
    double sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    for (int i = 0; i < n; i++) {
        double xi = x[i];
        sum1 += xi;
        sum2 += xi * xi;
        sum3 += xi * xi * xi;
        sum4 += xi * xi * xi * xi;
    }
    double mean = sum1 / n;

    printf("Raw moments about the origin:\n");
    printf("  mu1' : %12.4f\n", mean);
    printf("  mu2' : %12.4f\n", sum2 / n);
    printf("  mu3' : %12.4f\n", sum3 / n);
    printf("  mu4' : %12.4f\n\n", sum4 / n);

    /* STEP 4: central moments about the mean */
    double csum2 = 0, csum3 = 0, csum4 = 0;
    for (int i = 0; i < n; i++) {
        double diff = x[i] - mean;
        double diff2 = diff * diff;
        csum2 += diff2;
        csum3 += diff2 * diff;
        csum4 += diff2 * diff2;
    }
    double mu2 = csum2 / n;
    double mu3 = csum3 / n;
    double mu4 = csum4 / n;

    printf("Central moments about the mean (mean = %.4f):\n", mean);
    printf("  mu2  : %12.4f\n", mu2);
    printf("  mu3  : %12.4f\n", mu3);
    printf("  mu4  : %12.4f\n\n", mu4);

    if (mu2 == 0) {
        printf("All values are identical, so skewness and kurtosis are undefined.\n");
        return 0;
    }

    /* STEP 5: skewness and kurtosis */
    double gamma1 = mu3 / (mu2 * sqrt(mu2));   // mu2^1.5 without pow()
    double beta1 = gamma1 * gamma1;
    double beta2 = mu4 / (mu2 * mu2);
    double gamma2 = beta2 - 3.0;

    printf("Skewness and kurtosis:\n");
    printf("  gamma1 (skewness coefficient)  : %12.4f\n", gamma1);
    printf("  beta1  (= gamma1^2)            : %12.4f\n", beta1);
    printf("  beta2  (kurtosis coefficient)  : %12.4f\n", beta2);
    printf("  gamma2 (excess kurtosis)       : %12.4f\n\n", gamma2);

    /* STEP 6: interpretation */
    printf("Interpretation:\n");

    double abs_g1 = absolute(gamma1);
    if (abs_g1 < 0.5) {
        printf("  Skewness   : approximately symmetric (|gamma1| = %.4f < 0.5)\n", abs_g1);
    } else {
        char side[] = "left";
        if (gamma1 > 0) strcpy(side, "right");

        if (abs_g1 < 1.0)
            printf("  Skewness   : moderately %s-skewed (|gamma1| = %.4f)\n", side, abs_g1);
        else
            printf("  Skewness   : highly %s-skewed (|gamma1| = %.4f)\n", side, abs_g1);
    }

    if (absolute(gamma2) < 0.000001)
        printf("  Kurtosis   : mesokurtic (gamma2 = %.4f, approximately 0)\n", gamma2);
    else if (gamma2 > 0)
        printf("  Kurtosis   : leptokurtic (gamma2 = %.4f > 0, sharper peak / heavier tails)\n", gamma2);
    else
        printf("  Kurtosis   : platykurtic (gamma2 = %.4f < 0, flatter peak / lighter tails)\n", gamma2);

    return 0;
}
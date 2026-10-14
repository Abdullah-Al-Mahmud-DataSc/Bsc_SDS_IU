#include <stdio.h>
#include <stdlib.h>   // atof
#include <string.h>   // strcmp
#include <math.h>     // sqrt (needed for Pearson's r)

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

// Find the index of a column name in the header line (-1 if missing)
int find_column(char header[], char name[]) {
    char field[128];
    for (int col = 0; get_field(header, col, field); col++) {
        if (strcmp(field, name) == 0) return col;
    }
    return -1;
}

// Rough check that a field looks like a number (not empty / not text)
int looks_numeric(char field[]) {
    char c = field[0];
    return (c >= '0' && c <= '9') || c == '-' || c == '+' || c == '.';
}

double absolute(double x) {
    if (x < 0) return -x;
    return x;
}

int main() {
    char path[] = "../Ice_Cream.csv";

    /* STEP 1: open the file */
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("Error: could not open the file '%s'\n", path);
        return 1;
    }

    /* STEP 2: read the header and find both columns by name */
    char line[512];
    if (fgets(line, sizeof(line), file) == NULL) {
        printf("Error: the file is empty.\n");
        fclose(file);
        return 1;
    }

    int x_col = find_column(line, "Temperature");
    int y_col = find_column(line, "Revenue");
    if (x_col == -1 || y_col == -1) {
        printf("Error: 'Temperature' and/or 'Revenue' column not found\n");
        fclose(file);
        return 1;
    }

    /* STEP 3: read rows, skip bad ones, accumulate sums */
    int n = 0;
    int skipped = 0;
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0, sum_y2 = 0;
    char fx[128];
    char fy[128];

    while (fgets(line, sizeof(line), file) != NULL) {
        if (get_field(line, x_col, fx) && get_field(line, y_col, fy)
            && looks_numeric(fx) && looks_numeric(fy)) {

            double x = atof(fx);
            double y = atof(fy);

            n++;
            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
            sum_y2 += y * y;
        } else {
            skipped++;
        }
    }
    fclose(file);

    printf("Valid rows used : %d\n", n);
    printf("Rows skipped    : %d\n\n", skipped);

    if (n < 2) {
        printf("Not enough valid rows to compute a correlation.\n");
        return 1;
    }

    /* STEP 4: Pearson's r */
    double numerator = n * sum_xy - sum_x * sum_y;
    double denominator = sqrt((n * sum_x2 - sum_x * sum_x) *
                              (n * sum_y2 - sum_y * sum_y));

    if (denominator == 0) {
        printf("Correlation is undefined (no variation in Temperature or Revenue).\n");
        return 1;
    }

    double r = numerator / denominator;
    printf("Pearson correlation coefficient (r) : %.4f\n\n", r);

    /* STEP 5: classify strength and direction */
    double abs_r = absolute(r);

    printf("Relationship classification         : ");
    if (abs_r < 0.3) {
        printf("weak/none\n");
    } else {
        if (abs_r < 0.7)
            printf("moderate");
        else
            printf("strong");

        if (r > 0)
            printf(" positive\n");
        else
            printf(" negative\n");
    }

    return 0;
}
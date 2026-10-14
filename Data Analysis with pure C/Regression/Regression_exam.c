#include <stdio.h>
#include <math.h>

#define MAX_ROWS 5000
#define N_FEATURES 3
#define N_COLS (N_FEATURES + 1)

/* C = A * B */
void matmul(int rows, int inner, int cols,
            double A[rows][inner], double B[inner][cols], double C[rows][cols]) {

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {

            double sum = 0.0;

            for (int k = 0; k < inner; k++) {
                sum += A[i][k] * B[k][j];
            }

            C[i][j] = sum;
        }
    }
}

/* At = A^T */
void transpose(int rows, int cols,
               double A[rows][cols], double At[cols][rows]) {

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            At[j][i] = A[i][j];
        }
    }
}

/* determinant via cofactor expansion */
double determinant(int n, double A[n][n]) {

    if (n == 1) {
        return A[0][0];
    }

    if (n == 2) {
        return A[0][0] * A[1][1] - A[0][1] * A[1][0];
    }

    double det = 0.0;
    double minor[n - 1][n - 1];

    for (int col = 0; col < n; col++) {

        int mi = 0;
        for (int i = 1; i < n; i++) {
            int mj = 0;
            for (int j = 0; j < n; j++) {
                if (j == col) {
                    continue;
                }
                minor[mi][mj] = A[i][j];
                mj++;
            }
            mi++;
        }

        double sign = (col % 2 == 0) ? 1.0 : -1.0;
        det += sign * A[0][col] * determinant(n - 1, minor);
    }

    return det;
}

/* inverse via adjugate method; returns 0 on success, -1 if singular */
int invert(int n, double A[n][n], double Ainv[n][n]) {

    double det = determinant(n, A);

    if (fabs(det) < 1e-12) {
        printf("Error: matrix cannot be inverted (determinant is zero).\n");
        return -1;
    }

    double minor[n - 1][n - 1];

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {

            int mi = 0;
            for (int r = 0; r < n; r++) {
                if (r == i) {
                    continue;
                }
                int mj = 0;
                for (int c = 0; c < n; c++) {
                    if (c == j) {
                        continue;
                    }
                    minor[mi][mj] = A[r][c];
                    mj++;
                }
                mi++;
            }

            double sign = ((i + j) % 2 == 0) ? 1.0 : -1.0;
            double cofactor = sign * determinant(n - 1, minor);

            Ainv[j][i] = cofactor / det;
        }
    }

    return 0;
}

int main(void) {

    /* STEP 1: read the CSV file */
    // const char *path = "../student_performance_dataset.csv";
    const char *path = "D:\\Datasets\\student_performance_dataset.csv";
    FILE *file = fopen(path, "r");

    if (!file) {
        printf("Error: could not open the file '%s'\n", path);
        return 1;
    }

    static double X[MAX_ROWS][N_COLS];
    static double y[MAX_ROWS];
    int n = 0;

    char line[512];
    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file) && n < MAX_ROWS) {

        double study, attendance, sleep, score;

        int fields_found = sscanf(line, "%lf,%lf,%lf,%lf",
                                   &study, &attendance, &sleep, &score);

        if (fields_found == 4) {
            X[n][0] = 1.0;
            X[n][1] = study;
            X[n][2] = attendance;
            X[n][3] = sleep;
            y[n] = score;
            n++;
        }
    }

    fclose(file);
    printf("Loaded %d rows.\n\n", n);

    /* STEP 2: declare matrices */
    double Xt[N_COLS][n];
    double XtX[N_COLS][N_COLS];
    double XtXi[N_COLS][N_COLS];
    double Xty[N_COLS][1];
    double beta[N_COLS][1];
    double y_col[n][1];

    for (int i = 0; i < n; i++) {
        y_col[i][0] = y[i];
    }

    /* STEP 3: solve beta = (X^T*X)^-1 * X^T*y */
    transpose(n, N_COLS, X, Xt);
    matmul(N_COLS, n, N_COLS, Xt, X, XtX);
    matmul(N_COLS, n, 1, Xt, y_col, Xty);

    if (invert(N_COLS, XtX, XtXi) != 0) {
        return 1;
    }

    matmul(N_COLS, N_COLS, 1, XtXi, Xty, beta);

    /* STEP 4: show coefficients */
    printf("Fitted coefficients:\n");
    printf("  Intercept Beta0           : %10.4f\n", beta[0][0]);
    printf("  study_time_hours  Beta1   : %10.4f\n", beta[1][0]);
    printf("  attendance_percent   Beta2  : %10.4f\n", beta[2][0]);
    printf("  sleep_hours          Beta3  : %10.4f\n", beta[3][0]);

    /* STEP 5: evaluate model (R^2, RMSE) */
    double y_sum = 0.0;
    for (int i = 0; i < n; i++) {
        y_sum += y[i];
    }
    double y_mean = y_sum / n;

    double sum_squared_error = 0.0;
    double sum_squared_total = 0.0;

    for (int i = 0; i < n; i++) {

        double predicted = beta[0][0];
        for (int j = 1; j < N_COLS; j++) {
            predicted += beta[j][0] * X[i][j];
        }

        double error = y[i] - predicted;
        sum_squared_error += error * error;
        sum_squared_total += (y[i] - y_mean) * (y[i] - y_mean);
    }

    double r_squared = 1.0 - (sum_squared_error / sum_squared_total);
    double rmse = sqrt(sum_squared_error / n);

    printf("\nModel performance (full dataset):\n");
    printf("  R^2  : %.4f\n", r_squared);
    printf("  RMSE : %.4f\n", rmse);

    /* STEP 6: example prediction */
    double example_study = 5.0;
    double example_attendance = 90.0;
    double example_sleep = 7.0;

    double example_prediction = beta[0][0]
                               + beta[1][0] * example_study
                               + beta[2][0] * example_attendance
                               + beta[3][0] * example_sleep;

    printf("\nExample prediction (study=5h, attendance=90%%, sleep=7h):\n");
    printf("  Predicted final_exam_score = %.2f\n\n\n", example_prediction);

    return 0;
}
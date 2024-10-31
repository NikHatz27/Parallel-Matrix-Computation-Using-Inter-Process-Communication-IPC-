#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

// Function to genarate the matrix
void generate_matrix(int rows, int cols, int ***matrix);

// Function to generate a matrix with random values between 1 and 20
void generate_matrix(int rows, int cols, int ***matrix) {
    *matrix = (int **)malloc(rows * sizeof(int *));
    for (int i = 0; i < rows; i++) {
        (*matrix)[i] = (int *)malloc(cols * sizeof(int));
        for (int j = 0; j < cols; j++) {
            (*matrix)[i][j] = rand() % 20 + 1;
        }
    }
}

// Function to perform matrix multiplication for one element in the result matrix
void matrix_multiplication(int result_pipe_write, int **matrix_A, int **matrix_B, int row, int col, int cols_A) {
    int result = 0;
    for (int i = 0; i < cols_A; i++) {
        result += matrix_A[row][i] * matrix_B[i][col];
    }
    write(result_pipe_write, &result, sizeof(int)); // Send result to parent
    exit(0); // Child process exits after sending the result
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s <rows_A> <cols_A> <rows_B> <cols_B>\n", argv[0]);
        return 1;
    }
    srand(time(NULL)); // Seed for random number generation

    int rows_A = atoi(argv[1]);
    int cols_A = atoi(argv[2]);
    int rows_B = atoi(argv[3]);
    int cols_B = atoi(argv[4]);

    // Checking if the matrices can be multiplied (columns of A == rows of B)
    if (cols_A != rows_B) {
        printf("Number of columns in matrix A must equal number of rows in matrix B.\n");
        return 1;
    }

    int **matrix_A, **matrix_B;
    generate_matrix(rows_A, cols_A, &matrix_A); // Generate random matrix A
    generate_matrix(rows_B, cols_B, &matrix_B); // Generate random matrix B

    // Allocate memory for the result matrix
    int **result_matrix = (int **)malloc(rows_A * sizeof(int *));
    for (int i = 0; i < rows_A; i++) {
        result_matrix[i] = (int *)malloc(cols_B * sizeof(int));
    }

    // Perform matrix multiplication using the child process
    for (int i = 0; i < rows_A; i++) {
        for (int j = 0; j < cols_B; j++) {
            int pipes[2];
            pipe(pipes);

            pid_t pid = fork();
            if (pid == 0) { // Child process
                close(pipes[0]); // Close child reading end 
                matrix_multiplication(pipes[1], matrix_A, matrix_B, i, j, cols_A);
            } else if (pid > 0) { // Parent process
                close(pipes[1]); // Close paretn writing end 
                int result;
                read(pipes[0], &result, sizeof(int)); // Read result from child
                result_matrix[i][j] = result; // Store result in result_matrix
                close(pipes[0]); // Close parent reading end 
            } else {
                printf("Fork failed.\n");
                return 1;
            }
        }
    }

    // Print the result 
    printf("Result Matrix:\n");
    for (int i = 0; i < rows_A; i++) {
        for (int j = 0; j < cols_B; j++) {
            printf("%d ", result_matrix[i][j]);
        }
        printf("\n");
    }

    // Free allocated memory 
    for (int i = 0; i < rows_A; i++) free(matrix_A[i]);
    free(matrix_A);
    for (int i = 0; i < rows_B; i++) free(matrix_B[i]);
    free(matrix_B);
    for (int i = 0; i < rows_A; i++) free(result_matrix[i]);
    free(result_matrix);

    return 0;
}


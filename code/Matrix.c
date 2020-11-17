#include "Matrix.h"

void matmul(const char *tr, int32_t n, int32_t k, int32_t m, fp64 alpha, const fp64 *A, const fp64 *B, fp64 beta, fp64 *C)
{
    fp64 d;
    int32_t i, j, x, f = tr[0] == 'N' ? (tr[1] == 'N' ? 1 : 2) : (tr[1] == 'N' ? 3 : 4);

    for (i = 0; i < n; i++) for (j = 0; j < k; j++) {
        d = 0.0;
        switch (f)
        {
        case 1: for (x = 0; x < m; x++) d += A[i + x * n] * B[x + j * m]; break;
        case 2: for (x = 0; x < m; x++) d += A[i + x * n] * B[j + x * k]; break;
        case 3: for (x = 0; x < m; x++) d += A[x + i * m] * B[x + j * m]; break;
        case 4: for (x = 0; x < m; x++) d += A[x + i * m] * B[j + x * k]; break;
        }
        if (beta == 0.0) C[i + j * n] = alpha * d; else C[i + j * n] = alpha * d + beta * C[i + j * n];
    }
}

RETURN_STATUS matrix_init(matrix_t *matrix, const uint32_t row, const uint32_t col)
{
    if (row <= 0 || col <= 0)
    {
        // TODO: send warning to log
        return 0;
    }

    matrix->col = col;
    matrix->row = row;
    if (matrix->element = (fp64*)malloc(sizeof(fp64) * row * col))
    {
        matrix->is_valid = 1;
    }
    else
    {
        matrix->is_valid = 0;
        return 0;
    }
    
    memset(matrix->element, 0, sizeof(fp64) * col * row);

    return 1;
}
RETURN_STATUS matrix_free(matrix_t *matrix)
{
    if (matrix->row <= 0 || matrix->col <= 0 || matrix->is_valid == 0)
    {
        return 0;
    }

    free(matrix->element);

    matrix->row      = 0;
    matrix->col      = 0;
    matrix->element  = NULL;
    matrix->is_valid = 0;
    
    return 1;
}
RETURN_STATUS matrix_resize(matrix_t *matrix, uint32_t row, uint32_t col)
{
    if (matrix->row <= 0 || matrix->col <= 0)
    {
        return 0;
    }

    fp64 *temp = NULL;
    if (!(temp = (fp64*)realloc(matrix->element, sizeof(fp64) * row * col)))
    {
        return 0;
    }
    else
    {
        matrix->element  = temp;
        matrix->row      = row;
        matrix->col      = col;
        matrix->is_valid = 1;

        return 1;
    }
}

void matrix_print(matrix_t matrix)
{
    uint32_t i;
    uint32_t j;
    for (i = 0; i < matrix.row; ++i)
    {
        for (j = 0; j < matrix.col; ++j)
        {
            if (j == matrix.col - 1)
            {
                printf("%8.3f\n", matrix.element[i * matrix.col + j]);
            }
            else
            {
                printf("%8.3f", matrix.element[i * matrix.col + j]);
            }
        }
    }
}
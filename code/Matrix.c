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

static void trans_matrix_to_rtklib_mat(matrix_t *mat, fp64 *m)
{

}

static void trans_rtklib_mat_to_matrix(fp64 *m, uint32_t row, uint32_t col, matrix_t *mat)
{

}
RETURN_STATUS matrix_mlt(matrix_t *mat_in_1, matrix_t *mat_in_2, matrix_t *mat_out)
{
    if ((mat_in_1->row != mat_out->row) || (mat_in_1->col != mat_in_2->row) 
     || (mat_in_2->col != mat_out->col) || (!mat_in_1->is_valid) || (!mat_in_2->is_valid) 
     || (!mat_out->is_valid) || (mat_in_1->col <= 0) || (mat_in_1->row <= 0)
     || (mat_in_1->col <= 0) || (mat_in_2->col <=0))
     {
         // TODO: print log
         return RET_FAIL;
     }

    fp64 *rtklib_mat_in_1;
    fp64 *rtklib_mat_in_2;
    fp64 *rtklib_mat_out;
    if (!(rtklib_mat_in_1 = (fp64*)malloc(sizeof(fp64) * mat_in_1->col * mat_in_1->row)))
    {
        return RET_FAIL;
    }
    if (!(rtklib_mat_in_2 = (fp64*)malloc(sizeof(fp64) * mat_in_2->col * mat_in_2->row)))
    {
        return RET_FAIL;
    }
    if (!(rtklib_mat_out = (fp64*)malloc(sizeof(fp64) * mat_out->col * mat_out->row)))
    {
        return RET_FAIL;
    }
    trans_matrix_to_rtklib_mat(mat_in_1, rtklib_mat_in_1);
    trans_matrix_to_rtklib_mat(mat_in_2, rtklib_mat_in_2);
    trans_matrix_to_rtklib_mat(mat_out, rtklib_mat_out);

    matmul("NN", mat_in_1->row, mat_in_2->col, mat_in_1->col, 1.0, rtklib_mat_in_1, rtklib_mat_in_2, 0.0, rtklib_mat_out);

    trans_rtklib_mat_to_matrix(rtklib_mat_out, mat_out->row, mat_out->col, mat_out);

    return RET_SUCCESS;
}

RETURN_STATUS matrix_init(matrix_t *matrix, const uint32_t row, const uint32_t col)
{
    if (row <= 0 || col <= 0)
    {
        // TODO: send warning to log
        return RET_FAIL;
    }

    uint32_t i;
    uint32_t j;
    if (matrix->element = (fp64**)malloc(sizeof(fp64*) * row))
    {
        for (i = 0; i < row; ++i)
        {
            if (!(matrix->element[i] = (fp64*)malloc(sizeof(fp64) * col)))
            {
                for (j = 0; j < i; ++j)
                {
                    free(matrix->element[j]);
                }
                free(matrix->element);
                matrix->is_valid = 0;
                matrix->col = 0;
                matrix->row = 0;

                // TODO: print log
                return RET_FAIL;
            }
        }
    }
    else
    {
        matrix->col      = 0;
        matrix->row      = 0;
        matrix->is_valid = 0;
        // TODO: print log
        return RET_FAIL;
    }
    
    matrix->col      = col;
    matrix->row      = row;
    matrix->is_valid = 1;

    for (i = 0; i < row; ++i)
    {
        for (j = 0; j < col; ++j)
        {
            matrix->element[i][j] = 0.0;
        }
    }

    return RET_SUCCESS;
}
RETURN_STATUS matrix_free(matrix_t *matrix)
{
    if (matrix->row <= 0 || matrix->col <= 0 || matrix->is_valid == 0)
    {
        return RET_FAIL;
    }

    uint32_t i;
    uint32_t j;

    for (i = 0; i < matrix->row; ++i)
    {
        free(matrix->element[i]);
    }
    free(matrix->element);
    matrix->row      = 0;
    matrix->col      = 0;
    matrix->element  = NULL;
    matrix->is_valid = 0;
    
    return RET_SUCCESS;
}
extern RETURN_STATUS matrix_add(matrix_t *mat_in_1, matrix_t *mat_in_2, matrix_t *mat_out)
{
    if ((mat_in_1->row != mat_in_2->row) || (mat_in_1->col != mat_in_2->col) 
     || (mat_in_1->row != mat_out->row)  || (mat_in_1->col != mat_out->col) 
     || (!mat_in_1->is_valid) || (!mat_in_2->is_valid) || (!mat_out->is_valid)
     || (mat_in_1->col <= 0)  || (mat_in_1->row <= 0))
     {
         // TODO: print log
         return RET_FAIL;
     }

     uint32_t i;
     uint32_t j;
     for (i = 0; i < mat_out->row; ++i)
     {
         for (j = 0; j <mat_out->col; ++j)
         {
             mat_out->element[i][j] = mat_in_1->element[i][j] + mat_in_2->element[i][j];
         }
     }

     return RET_SUCCESS;
}

extern RETURN_STATUS matrix_miu(matrix_t *mat_in_1, matrix_t *mat_in_2, matrix_t *mat_out)
{
    if ((mat_in_1->row != mat_in_2->row) || (mat_in_1->col != mat_in_2->col) 
     || (mat_in_1->row != mat_out->row)  || (mat_in_1->col != mat_out->col) 
     || (!mat_in_1->is_valid) || (!mat_in_2->is_valid) || (!mat_out->is_valid)
     || (mat_in_1->col <= 0)  || (mat_in_1->row <= 0))
     {
         // TODO: print log
         return RET_FAIL;
     }

     uint32_t i;
     uint32_t j;
     for (i = 0; i < mat_out->row; ++i)
     {
         for (j = 0; j <mat_out->col; ++j)
         {
             mat_out->element[i][j] = mat_in_1->element[i][j] - mat_in_2->element[i][j];
         }
     }

     return RET_SUCCESS;
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
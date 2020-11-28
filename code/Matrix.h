#include "common.h"
/*
*@brief Matrix struct
*@note  is_valid means whether Matrix is ready to use or not
*/
typedef struct
{
    uint32_t row;
    uint32_t col;
    uint8_t  is_valid;
    fp64     **element;
} matrix_t;

/*
 *@brief            multiply matrix
 *@param[in]        tr   : transpose flag. the first char means the first input matrix transpose state,
 *                         the second char means the second input matrix transpose state
 *@param[in]        n    : row size of matrix A
 *@param[in]        k    : col size of matrix B
 *@param[in]        m    : col/row size of matrix A/B
 *@param[in]        alpha: mlutiply factor
 *@param[in]        A    : front matrix
 *@param[in]        B    : back matrix
 *@param[in]        beta : mlutiply factor
 *@param[in/out]    C    : multiply matrix result
 *@retval           none
 *@note             if tr = "NN", C = alpha * (A  * B)  + beta * C
 *                  if tr = "TN", C = alpha * (AT * B)  + beta * C
 *                  if tr = "NT", C = alpha * (A  * BT) + beta * C
 *                  if tr = "TT", C = alpha * (AT * BT) + beta * C
 **/
extern void matmul(const char* tr, int32_t n, int32_t k, int32_t m, fp64 alpha, const fp64 *A, 
                   const fp64 *B, fp64 beta, fp64 *C);

extern int32_t matinv(fp64 *A, int32_t n);

extern RETURN_STATUS matrix_init(matrix_t *matrix, const uint32_t row, const uint32_t col);

extern RETURN_STATUS matrix_resize(matrix_t *mat, const uint32_t row, const uint32_t col);

extern RETURN_STATUS matrix_free(matrix_t *matrix);

extern RETURN_STATUS matrix_mlt(matrix_t *mat_in_1, matrix_t *mat_in_2, matrix_t *mat_out);

extern RETURN_STATUS matrix_trs(matrix_t *mat_in, matrix_t *mat_out);

extern RETURN_STATUS matrix_inv(matrix_t *mat_in, matrix_t *mat_out);

extern RETURN_STATUS matrix_add(matrix_t *mat_in_1, matrix_t *mat_in_2, matrix_t *mat_out);

extern RETURN_STATUS matrix_miu(matrix_t *mat_in_1, matrix_t *mat_in_2, matrix_t *mat_out);

extern RETURN_STATUS matrix_extend_one_col(matrix_t *mat);

extern RETURN_STATUS matrix_copy(const matrix_t *mat_in, matrix_t *mat_out);

extern void matrix_print(matrix_t matrix);


/**
  ***************************************(C) COPYRIGHT 2020 Wyatt Wu***********************************
  * @file        lib.c/h
  * @brief       This head/source file is used to write various basic library functions(mainly  
  *              from RTKLIB), basic macro definitions, and clear definitions of basic types
  * @note
  * @history
  * Version      Date            Author          Modification
  * V1.0.0       Nov-07-2020     Wyatt Wu        1. build this file and some foundational content.
  ***************************************(C) COPYRIGHT 2020 Wyatt Wu***********************************
*/
#pragma once
#include <math.h>

#define OMGE        (7.2921151467E-5)       /* earth angular velocity (IS-GPS) (rad/s)*/
#define FE_WGS84    (1.0/298.257223563)     /* earth flattening (WGS84) */
#define RE_WGS84    (6378137.0)             /* earth semimajor axis (WGS84) (m)*/
#define PI          (3.1415926535897932)    /* pi */
#define CLIGHT      (299792458.0)           /* light speed */

/* redefine data type to adapt other platform */
typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef signed short        int16_t;
typedef unsigned short      uint16_t;
typedef int                 int32_t;
typedef unsigned int        uint32_t;
typedef long long           int64_t;
typedef unsigned long long  uint64_t;
typedef float               fp32;
typedef double              fp64;
typedef unsigned char       bool_t;
typedef unsigned char       RETURN_STATUS;

/**
  * @brief      convert calendar day/time to time
  * @author     quote from RTKLIB, modified by Wyatt wu
  * @param[in]  ep: day/time {year,month,day,hour,min,sec}
  * @retval     fp64 time
 */
extern fp64 epoch2time(const fp64 *ep);

/**
  * @brief      time to calendar day/time 
  * @author     quote from RTKLIB, modified by Wyatt wu
  * @param[in]  time: 
  * @param[out] ep  : day/time {year,month,day,hour,min,sec}
 */
extern void time2epoch(fp64 time, fp64 *ep);


/**
  * @brief         convert time to week and tow in gps time
  * @author        quote from RTKLIB, modified by Wyatt wu
  * @param[in]     t    : gps time in 1970
  * @param[in/out] week : week number in gps time (NULL: no output)
  * @retval        time of week in gps time (s)
 */
extern fp64 time2gpst(fp64 t, int32_t *week);

/* 
 * @brief        inner product of vectors
 * @auther       quote from RTKLIB
 * @param[in]    a:vector a (n x 1)
 * @param[in]    b:vector b (n x 1)
 * @param[in]    n: size of vector a,b
 * @retval       a'*b
 *-----------------------------------------------------------------------------
*/
extern fp64 dot(const fp64 *a, const fp64 *b, int32_t n);

/*
 *@brief        euclid norm of vector
 *@auther       quote from RTKLIB
 *@param[in]    a : vector a (n x 1)
 *@param[in]    n : size of vector a
 *@retval       || a ||
 **/
extern fp64 norm(const fp64* a, int32_t n);

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
extern void matmul(const char *tr, int32_t n, int32_t k, int32_t m, fp64 alpha,const fp64 *A, const fp64 *B, fp64 beta, fp64 *C);
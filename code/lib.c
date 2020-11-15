#pragma once
#include "lib.h"

fp64 epoch2time(const fp64 *ep)
{
    const int doy[] = { 1,32,60,91,121,152,182,213,244,274,305,335 };
    fp64 time = 0;
    int32_t days, year = (int32_t)ep[0], mon = (int32_t)ep[1], day = (int32_t)ep[2];

    if (year < 1970 || 2099 < year || mon < 1 || 12 < mon) return time;

    /* leap year if year%4==0 in 1901-2099 */
    days = (year - 1970) * 365 + (year - 1969) / 4 + doy[mon - 1] + day - 2 + (year % 4 == 0 && mon >= 3 ? 1 : 0);

    time = (fp64)days * 86400 + ep[3] * 3600 + ep[4] * 60 + ep[5];

    return time;
}

void time2epoch(fp64 time, fp64 *ep)
{
    const int mday[] = { /* # of days in a month */
        31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31,
        31,29,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31
    };
    int32_t days, sec, mon, day;

    fp64 fra = time - floor(time);

    /* leap year if year%4==0 in 1901-2099 */
    days = (int)(time / 86400);
    sec = (int)(time - (time_t)days * 86400);
    for (day = days % 1461, mon = 0; mon < 48; mon++) {
        if (day >= mday[mon]) day -= mday[mon]; else break;
    }
    ep[0] = 1970 + (fp64)days / 1461 * 4 + (fp64)mon / 12;
    ep[1] = (fp64)(mon % 12) + 1;
    ep[2] = (fp64)day + 1;
    ep[3] = sec / 3600;
    ep[4] = sec % 3600 / 60;
    ep[5] = sec % 60 + fra;
}


fp64 dot(const fp64 *a, const fp64 *b, int32_t n)
{
    fp64 c = 0.0;

    while (--n >= 0) c += a[n] * b[n];
    return c;
}

fp64 norm(const fp64 *a, int32_t n)
{
    return sqrt(dot(a, a, n));
}

void matmul(const char *tr, int32_t n, int32_t k, int32_t m, fp64 alpha,const fp64 *A, const fp64 *B, fp64 beta, fp64 *C)
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
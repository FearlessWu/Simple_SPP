#pragma once
#include "lib.h"


fp64 epoch2time(const double* ep)
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
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
extern void time2epoch(fp64 time, fp64 *ep)
{
    const int mday[] = { /* # of days in a month */
        31,28,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31,
        31,29,31,30,31,30,31,31,30,31,30,31,31,28,31,30,31,30,31,31,30,31,30,31
    };
    int days, sec, mon, day;

    fp64 fra = time - floor(time);

    /* leap year if year%4==0 in 1901-2099 */
    days = (int)(time / 86400);//将天化为秒为单位
    sec = (int)(time - (time_t)days * 86400);//溢出秒数
    for (day = days % 1461, mon = 0; mon < 48; mon++) {
        if (day >= mday[mon]) day -= mday[mon]; else break;
    }
    ep[0] = 1970 + days / 1461 * 4 + mon / 12;
    ep[1] = mon % 12 + 1;
    ep[2] = day + 1;
    ep[3] = sec / 3600;
    ep[4] = sec % 3600 / 60;
    ep[5] = sec % 60 + fra;
}
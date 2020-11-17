
#include "lib.h"

static const fp64 gpst0[6] = { 1980,1, 6,0,0,0 }; /* gps time reference */

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

fp64 time2gpst(fp64 t, int32_t *week)
{
    fp64 t0 = epoch2time(gpst0);
    fp64 sec = t - t0;
    int32_t w = (int32_t)((int32_t)sec / (86400 * 7));

    if (week) *week = w;
    return (fp64)(sec - (fp64)w * 86400 * 7);
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


void xyz2blh(const fp64* xyz, fp64* blh)
{
    fp64 z;
    fp64 zk;
    fp64 sinp;
    fp64 v = RE_WGS84;
    fp64 e2 = FE_WGS84 * (2.0 - FE_WGS84);
    fp64 r2 = dot(xyz, xyz, 2);

    for (z = xyz[2], zk = 0.0; fabs(z - zk) >= 1E-4;)
    {
        zk = z;
        sinp = z / sqrt(r2 + z * z);
        v = RE_WGS84 / sqrt(1.0 - e2 * sinp * sinp);
        z = xyz[2] + v * e2 * sinp;
    }

    blh[0] = r2 > 1E-12 ? atan(z / sqrt(r2)) : (xyz[2] > 0.0 ? PI / 2.0 : -PI / 2.0);
    blh[1] = r2 > 1E-12 ? atan2(xyz[1], xyz[0]) : 0.0;
    blh[2] = sqrt(r2 + z * z) - v;
}


fp64 geodist(const fp64* rs, const fp64* rr, fp64* e)
{
    fp64  r;
    int32_t i;

    if (norm(rs, 3) < RE_WGS84) return -1.0;
    for (i = 0; i < 3; i++) e[i] = rs[i] - rr[i];
    r = norm(e, 3);
    for (i = 0; i < 3; i++) e[i] /= r;
    return r + OMGE * (rs[0] * rr[1] - rs[1] * rr[0]) / CLIGHT;
}

void xyz2enu(const fp64 *blh, fp64 *E)
{
    fp64 sinp = sin(blh[0]); 
    fp64 cosp = cos(blh[0]); 
    fp64 sinl = sin(blh[1]); 
    fp64 cosl = cos(blh[1]);

    E[0] = -sinl;         E[3] = cosl;         E[6] = 0.0;
    E[1] = -sinp * cosl;  E[4] = -sinp * sinl; E[7] = cosp;
    E[2] = cosp  * cosl;  E[5] = cosp  * sinl; E[8] = sinp;
}

void ecef2enu(const fp64 *blh, const fp64 *r, fp64 *e)
{
    fp64 E[9];

    xyz2enu(blh, E);
    matmul("NN", 3, 1, 3, 1.0, E, r, 0.0, e);
}

fp64 satazel(const fp64 *blh, const fp64 *e, fp64 *azel)
{
    fp64  az = 0.0;
    fp64  el = PI / 2.0;
    fp64  enu[3];

    if (blh[2] > -RE_WGS84)
    {
        ecef2enu(blh, e, enu);
        az = dot(enu, enu, 2) < 1E-12 ? 0.0 : atan2(enu[0], enu[1]);
        if (az < 0.0) az += 2 * PI;
        el = asin(enu[2]);
    }
    if (azel) { azel[0] = az; azel[1] = el; }

    return el;
}
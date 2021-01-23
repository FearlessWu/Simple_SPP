
#include "spp_sol.h"
#include "io.h"
#define BROADCAST_EPH_THRESHOLD    (7200)                //unit: sec
#define GPS_GM                     (3.986004418E14)      // earth-gravitational constant reference to RTKLIB
#define ERR_BRDCI                  (0.5)                 /* broadcast iono model error factor */

/* global variable */
FILE        *obs_fp_ptr;
extern log_t loger;

/* variable only use in this file */
obs_epoch_t   obs_c;
eph_t         eph;

eph_sat_t sel_broadcast_eph(fp64 time, int32_t sys_id, int32_t sat_id, eph_t *eph)
{
    eph_sat_t eph_sat = { 0 };
    if (!(sys_id & SYS_GPS))
    {
        return eph_sat;
    }

    int32_t i;
    for (i = 0; i < MAXGPSNUM; ++i)
    {
        if (sat_id != eph->gps_eph[i][0].sv_id)
        {
            continue;
        }

        int32_t j;
        int32_t backward_idx;
        fp64    backward_time = time - VERY_BIG_NUM;
        fp64    eph_time;

        for (j = 0; j < MAXBROEPH; ++j)
        {
            if (eph->gps_eph[i][j].time[0] == 0)
            {
                continue;
            }
            fp64 ep[6] = { 0 };
            int32_t k;
            for (k = 0; k < 5; k++)
            {
                ep[k] = eph->gps_eph[i][j].time[k];
            }

            eph_time = epoch2time(ep);

            if (eph_time == time)
            {
                return eph->gps_eph[i][j];
            }

            if (eph_time < time)
            {
                if (eph_time > backward_time)
                {
                    backward_idx = j;
                    backward_time = eph_time;

                    continue;
                }
            }
            if (eph_time > time)
            {
                continue;
            }
        }
        if ((time - backward_time) < BROADCAST_EPH_THRESHOLD)
        {
            return eph->gps_eph[i][backward_idx];
        }
    }
    return eph_sat;
}

RETURN_STATUS get_sat_pos_broadcast_eph(eph_sat_t *eph_sat, fp64 *sat_pos, fp64 *sat_clk, fp64 time, fp64 *var)
{
    if (!eph_sat->nav_valid || (eph_sat->sv_hea))
    {
        // TODO: do something
        return RET_FAIL;
    }

    fp64 E;
    fp64 M;
    fp64 u;
    fp64 tk;
    fp64 cos2u;
    fp64 sin2u;
    fp64 r;
    fp64 x;
    fp64 y;
    fp64 eph_i;
    fp64 omg_tk;
    uint32_t i;
    
    tk = time - *sat_clk - eph_sat->Toc;
    M  = (sqrt(GPS_GM) / (eph_sat->rootA * eph_sat->rootA * eph_sat->rootA) + eph_sat->DeltaN) * tk + eph_sat->M0;

    /* Iterative calculation of near point angle */
    i = 0;
    E = M;
    fp64 last_E = VERY_BIG_NUM;
    while (1)
    {
        if (i > 30)
        {
            // TODO: do something
            return RET_FAIL;
        }
        E = M + eph_sat->E * sin(E);
        if (fabs(E - last_E) < 1E-13)
        {
            break;
        }
        last_E = E;
        ++i;
    }
    u      = atan2(sqrt(1.0 - eph_sat->E * eph_sat->E) * sin(E), cos(E) - eph_sat->E) + eph_sat->Omega;
    cos2u = cos(2 * u);
    sin2u = sin(2 * u);

    u      += eph_sat->CuC * cos2u + eph_sat->CuS * sin2u;
    r       = eph_sat->rootA * eph_sat->rootA * (1 - eph_sat->E * cos(E)) + eph_sat->CrC * cos2u + eph_sat->CrS * sin2u;
    eph_i   = eph_sat->I0 + eph_sat->CiC * cos2u + eph_sat->CiS * sin2u + eph_sat->Idot * tk;
    x       = r * cos(u);
    y       = r * sin(u);
    omg_tk  = eph_sat->Omega0 + (eph_sat->OmegaDot - OMGE) * tk - OMGE * eph_sat->Toe;
 
    sat_pos[0] = x * cos(omg_tk) - y * cos(eph_i) * sin(omg_tk);
    sat_pos[1] = x * sin(omg_tk) + y * cos(eph_i) * cos(omg_tk);
    sat_pos[2] = y * sin(eph_i);

    /* relativity correction */
    tk        = time - eph_sat->Toc;
    *sat_clk  = eph_sat->sv_clk[0] + eph_sat->sv_clk[1] * tk + eph_sat->sv_clk[2] * tk * tk;
    *sat_clk -= 2.0 * sqrt(GPS_GM * eph_sat->rootA * eph_sat->rootA) * eph_sat->E * sin(E) / CLIGHT / CLIGHT;

    /* calculate sv variance */
    if (0 <= eph_sat->sv_acc && eph_sat->sv_acc <= 6)
    {
        *var = pow(2.0, (1.0 + eph_sat->sv_acc / 2.0));
    }
    else if (7 <= eph_sat->sv_acc && eph_sat->sv_acc <= 15)
    {
        *var = pow(2.0, (eph_sat->sv_acc - 2));
    }
    else
    {
        *var = 8192;
    }

    return RET_SUCCESS;
}

fp64 get_sv_clk_broadcast_eph(obs_epoch_t *obs_c, eph_sat_t *eph_sat, sat_info_t *sat_info)
{
    fp64 delta_t;
    int32_t k;
    int32_t m;
    fp64 ep[6] = { 0 };

    fp64 eph_time;
    for (k = 0; k < 5; k++)
    {
        ep[k] = eph_sat->time[k];
    }

    eph_time = epoch2time(ep);

    /* save Toc */
    eph_sat->Toc = eph_time;

    for (k = 0; k < obs_c->sv_num; ++k)
    {
        if (eph_sat->sv_id == obs_c->obs[k].sv_id && eph_sat->sys_id == obs_c->obs[k].sys_id)
        {
            break;
        }
    }
    if (k == obs_c->sv_num)
    {
        // TODO: do something
        return RET_FAIL;
    }

    delta_t = obs_c->time - (obs_c->obs[k].P[0] / CLIGHT) - eph_time;

    for (m = 0; m < MAXGPSNUM; m++)
    {
        if (eph_sat->sv_id == sat_info->gps_sat[m].sv_id)
        {
            break;
        }
    }
    if (m == MAXGPSNUM)
    {
        // TODO: do something
        return RET_FAIL;
    }

    for (k = 0; k < 2; ++k)
    {
        delta_t -= eph_sat->sv_clk[0] + eph_sat->sv_clk[1] * delta_t + eph_sat->sv_clk[2] * delta_t * delta_t;
    }

    return (eph_sat->sv_clk[0] + eph_sat->sv_clk[1] * delta_t + eph_sat->sv_clk[2] * delta_t * delta_t);
    
}

static fp64 tropo_param_interpolation(fp64 phi, fp64 phi1, fp64 phi0, fp64 p1, fp64 p0)
{
    fp64 temp = (phi - phi0);
    temp = temp / (phi1 - phi0);
    temp = temp * (p1 - p0);
    temp += p0;
    return temp;
}

static fp64 get_tropo_param(int32_t index, fp64 phi_degree, int32_t doy)
{
    fp64 p0;
    fp64 PDot;
    fp64 temp;
    uint8_t DayRef = 28;

    const  fp64 TropoTable0[5][5] = {
        {1013.25,    299.65,    26.31,    0.00630,    2.77},
        {1017.25,    294.15,    21.79,    0.00605,    3.15},
        {1015.75,    283.15,    11.66,    0.00558,    2.57},
        {1011.75,    272.15,     6.78,    0.00539,    1.81},
        {1013.00,    263.65,     4.11,    0.00453,    1.55} };

    const  fp64 TropoTable1[5][5] = {
        {0,        0,      0,    0,         0   },
        {-3.75,    7.0,  8.85,    0.00025, 0.33},
        {-2.25,    11.0, 7.24,    0.00032, 0.46},
        {-1.75,    15.0, 5.36,    0.00081, 0.74},
        {-0.50,    14.5, 3.39,    0.00062, 0.30} };

    if (phi_degree < 0)
    {
        DayRef     = 211;
        phi_degree = -1 * phi_degree;
    }

    if (phi_degree < 15)
    {
        p0   = TropoTable0[0][index];
        PDot = TropoTable1[0][index];
    }
    else if (phi_degree > 75)
    {
        p0   = TropoTable0[4][index];
        PDot = TropoTable1[4][index];
    }
    else
    {
        int32_t phiIndex;
        fp64 phi1_1, phi0_1;
        phiIndex = (int)(phi_degree / 15.) - 1;
        phi0_1 = ((fp64)phiIndex + 1) * 15;
        phi1_1 = phi0_1 + 15;
        p0 = tropo_param_interpolation(phi_degree, phi1_1, phi0_1, TropoTable0[phiIndex + 1][index], TropoTable0[phiIndex][index]);
        PDot = tropo_param_interpolation(phi_degree, phi1_1, phi0_1, TropoTable1[phiIndex + 1][index], TropoTable1[phiIndex][index]);
    }

    temp = 2 * PI *((fp64)doy - DayRef) / 365.25;
    temp = cos(temp);
    temp = temp * PDot;
    temp = p0 - temp;
    return temp;
}

static fp64 mops_tropo_delay(fp64 lat, fp64 h, fp64 ele, int32_t doy)
{
    fp64 temp = ele;
    fp64 SinEl = sin(temp);
    fp64 M;
    fp64 d_dry = 0.0, d_wet = 0.0;
    fp64 OneMBetaH, Rdp10_6, gDivRdBeta;
    fp64 result = 0.0;

    const fp64 k1 = 77.604;
    const fp64 k2 = 382000;
    const fp64 Rd = 287.054;
    const fp64 gm = 9.784;
    const fp64 g  = 9.80665;
    const fp64 SpeedOfLight = 299792458.0;
    
    fp64 p, T, e, beta, lambda;
    fp64  Lat_degree = (lat) * 180 / PI;

    p =      get_tropo_param(0, Lat_degree, doy);
    T =      get_tropo_param(1, Lat_degree, doy);
    e =      get_tropo_param(2, Lat_degree, doy);
    beta =   get_tropo_param(3, Lat_degree, doy);
    lambda = get_tropo_param(4, Lat_degree, doy);

    OneMBetaH = (1.0 - beta * h / T);
    Rdp10_6 = 1E-6 * Rd * p;
    gDivRdBeta = g / Rd / beta;
    d_dry = pow(OneMBetaH, gDivRdBeta) * (Rdp10_6 * k1 / gm);
    d_wet = pow(OneMBetaH, ((lambda + 1.0) * gDivRdBeta - 1.)) * (Rdp10_6 / p * k2 * e / T / (gm * (lambda + 1) - beta * Rd));
    M = 1.001 / sqrt(0.002001 + SinEl * SinEl);

    result = ((d_dry + d_wet) * M / SpeedOfLight);

    if (isnan(result) || isinf(result) || fabs(result) > 1000.)
    {
        return 0;
    }    
    else
    {
        return result;
    }
}

static void earth_rotate_corr(sat_info_t* sat_info, uint32_t sv_id, uint32_t sys_id, rcv_info_t* rcv_info)
{
    fp64 dx, dy, dz;
    fp64 RotateAngle;
    fp64 sin_temp, cos_temp, temp;

    const fp64 WGS_OMEGDOTE = 7.2921151467e-5;
    if (sys_id == SYS_GPS)
    {
        dx = sat_info->gps_sat[sv_id - 1].satpos[0] - rcv_info->appro_pos[0];
        dy = sat_info->gps_sat[sv_id - 1].satpos[1] - rcv_info->appro_pos[1];
        dz = sat_info->gps_sat[sv_id - 1].satpos[2] - rcv_info->appro_pos[2];
        dx = (fp64)sqrt(dx * dx + dy * dy + dz * dz);
        dx = dx / CLIGHT;
        RotateAngle = dx * WGS_OMEGDOTE;
        sin_temp = sin(RotateAngle);
        cos_temp = cos(RotateAngle);
        temp = sat_info->gps_sat[sv_id - 1].satpos[0] * cos_temp + sat_info->gps_sat[sv_id - 1].satpos[1] * sin_temp;
        sat_info->gps_sat[sv_id - 1].satpos[1] = sat_info->gps_sat[sv_id - 1].satpos[1] * cos_temp - sat_info->gps_sat[sv_id - 1].satpos[0] * sin_temp;
        sat_info->gps_sat[sv_id - 1].satpos[0] = temp;
        temp = sat_info->gps_sat[sv_id - 1].satvel[0] * cos_temp + sat_info->gps_sat[sv_id - 1].satvel[1] * sin_temp;
        sat_info->gps_sat[sv_id - 1].satvel[1] = sat_info->gps_sat[sv_id - 1].satvel[1] * cos_temp - sat_info->gps_sat[sv_id - 1].satvel[0] * sin_temp;
        sat_info->gps_sat[sv_id - 1].satvel[0] = temp;
    }
}

static fp64 broadcast_iono_delay(fp64 time, fp64 *blh, obs_sv_t *obs, sat_info_t *sat_info)
{
    uint8_t i = 0;
    const fp64* azel = obs->azel;
    fp64 f, amp, per, x;
    fp64 sc, fi, lambda;
    fp64 tt;
    int32_t week;
    sin_sys_ion_cor_t *klbr_iono_param = &sat_info->sys_ion_cor.gps_ino_cor;

    /* if no broadcast ionosphere parameters, using default parameters instead */
    if (norm(klbr_iono_param->alph, 4) * norm(klbr_iono_param->beta, 4) <= 0)
    {
        klbr_iono_param->alph[0] =  0.1118E-07;
        klbr_iono_param->alph[1] = -0.7451E-08;
        klbr_iono_param->alph[2] = -0.5961E-07;
        klbr_iono_param->alph[3] =  0.1192E-06;
        klbr_iono_param->beta[0] =  0.1167E+06;
        klbr_iono_param->beta[1] = -0.2294E+06;
        klbr_iono_param->beta[2] = -0.1311E+06;
        klbr_iono_param->beta[3] =  0.1049E+07;
    }
   
    if (blh[2] < -1e3 || azel[1] <= 0)
    {
        return 0;
    }

    /* earth centered angle (semi-circle) */
    sc = 0.0137 / (azel[1] / PI + 0.11) - 0.022;

    /* subionospheric latitude/longitude (semi-circle) */
    fi = blh[0] / PI + sc * cos(azel[0]);
    if (fi > 0.416)
    {
        fi = 0.416;
    }
    else if (fi < -0.416)
    {
        fi = -0.416;
    }
    lambda = blh[1] / PI + sc * sin(azel[0]) / cos(fi * PI);

    /* geomagnetic latitude (semi-circle) */
    fi += 0.064 * cos((lambda - 1.617) * PI);
    
    /* local time (s) */
    tt = 43200.0 * lambda + time2gpst(time, &week);
    tt -= floor(tt / 86400.0) * 86400.0; /* 0<=tt<86400 */
    
    /* slant factor */
f = 1.0 + 16.0 * pow(0.53 - azel[1] / PI, 3.0);

/* ionospheric delay */
amp = klbr_iono_param->alph[0] + fi * (klbr_iono_param->alph[1] + fi * (klbr_iono_param->alph[2] + fi * klbr_iono_param->alph[3]));
per = klbr_iono_param->beta[0] + fi * (klbr_iono_param->beta[1] + fi * (klbr_iono_param->beta[2] + fi * klbr_iono_param->beta[3]));
amp = amp < 0.0 ? 0.0 : amp;
per = per < 72000.0 ? 72000.0 : per;
x = 2.0 * PI * (tt - 50400.0) / per;

return CLIGHT * f * (fabs(x) < 1.57 ? 5E-9 + amp * (1.0 + x * x * (-0.5 + x * x / 24.0)) : 5E-9);
}

RETURN_STATUS get_sv_pos_clk(obs_epoch_t* obs_c, eph_t* eph, sat_info_t* sat_info)
{
    int32_t i = 0;
    for (i = 0; i < obs_c->sv_num; ++i)
    {
        eph_sat_t eph_sat;
        fp64 sat_clk = 0;
        fp64 sat_pos[3] = { 0 };
        fp64 time_c;
        fp64 time_s;
        fp64 var;
        fp64 var_s;

        eph_sat = sel_broadcast_eph(obs_c->time, obs_c->obs[i].sys_id, obs_c->obs[i].sv_id, eph);
        if (eph_sat.time[0] == 0)
        {
            // TODO: do something
            return false;
        }

        sat_clk = get_sv_clk_broadcast_eph(obs_c, &eph_sat, sat_info);
        time_c = obs_c->time - obs_c->obs[i].P[0] / CLIGHT;
        get_sat_pos_broadcast_eph(&eph_sat, sat_pos, &sat_clk, time_c, &var);
        for (int32_t j = 0; j < 3; ++j)
        {
            sat_info->gps_sat[obs_c->obs[i].sv_id - 1].satpos[j] = sat_pos[j];
        }
        sat_info->gps_sat[obs_c->obs[i].sv_id - 1].satclk[0] = sat_clk;
        sat_info->gps_sat[obs_c->obs[i].sv_id - 1].pos_var = var;

        /* calculate satellite velocity and clock drift */
        time_s = time_c - 1E-3;
        sat_clk = get_sv_clk_broadcast_eph(obs_c, &eph_sat, sat_info);
        get_sat_pos_broadcast_eph(&eph_sat, sat_pos, &sat_clk, time_s, &var_s);
        for (int32_t j = 0; j < 3; ++j)
        {
            sat_info->gps_sat[obs_c->obs[i].sv_id - 1].satvel[j] = (sat_info->gps_sat[obs_c->obs[i].sv_id - 1].satpos[j] - sat_pos[j]) / 1E-3;
        }
        sat_info->gps_sat[obs_c->obs[i].sv_id - 1].satclk[1] = (sat_info->gps_sat[obs_c->obs[i].sv_id - 1].satclk[0] - sat_clk) / 1E-3;
        sat_info->gps_sat[obs_c->obs[i].sv_id - 1].is_vaild = true;
    }

    return RET_SUCCESS;
}

static void init_sat_info(sat_info_t* sat_info)
{
    int32_t i;
    memset(sat_info, 0, sizeof(sat_info_t));
    for (i = 0; i < MAXGPSNUM; ++i)
    {
        sat_info->gps_sat[i].sys_id = SYS_GPS;
        sat_info->gps_sat[i].sv_id = i + 1;
        sat_info->gps_sat[i].is_vaild = false;
    }
}

static int32_t pre_residual_deterct(obs_sv_t* obs, sat_info_t* sat_info)
{
    int32_t obs_num = 0;

    return obs_num;
}

static uint8_t get_rs_clk(const uint32_t sys_id, const uint32_t sv_id, fp64 *rs, fp64 *sv, fp64 *sat_clk, const sat_info_t *sat_info)
{
    uint32_t i;
    switch (sys_id)
    {
    case SYS_GPS:
        if (!sat_info->gps_sat[sv_id - 1].is_vaild)
        {
            return false;
        }
        for (i = 0; i < 3; ++i)
        {
            rs[i] = sat_info->gps_sat[sv_id - 1].satpos[i];
            sv[i] = sat_info->gps_sat[sv_id - 1].satvel[i];
            if (i < 2)
            {
                sat_clk[i] = sat_info->gps_sat[sv_id - 1].satclk[i];
            }
        }
        break;
    default:
        break;
    }

    return true;
}

static fp64 get_psdrnge_var(fp64 el, int32_t sys)
{
    fp64  var = VERY_BIG_NUM;
    if (sys & SYS_GPS)
    {
        var = SQR(100.0) * (SQR(0.003) + SQR(0.003) / sin(el));
    }
    return var;
}

static int32_t construct_H_R_V_matrix(obs_epoch_t *obs_c, sat_info_t *sat_info, const spp_sol_t *spp_sol, matrix_t *H, 
                                      matrix_t *R, matrix_t *v, uint8_t spp_init, const fp64 *appro_param)
{
    int32_t i;
    int32_t sv_num = 0;
    int32_t est_num;

    fp64 r      = 0;
    fp64 lam    = 0;
    fp64 dtr    = 0;
    fp64 dft    = 0;
    fp64 e[3]   = { 0 };
    fp64 blh[3] = { 0 };
    fp64 xyz[3] = { 0 };
    fp64 vel[3] = { 0 };
    
    matrix_t R_tmp;

    ESTIMATE_PARAM_NUM(est_num);
    matrix_init(H, 2, est_num);
    matrix_init(v, 2, 1);
    matrix_init(&R_tmp, 2, 1);

    for (i = 0; i < 3; ++i)
    {
        xyz[i] = appro_param[i];
        vel[i] = appro_param[i + POS_PARAM_NUM + DTR_PARAM_NUM];
    }

    dtr = appro_param[3];
    dft = appro_param[7];

    xyz2blh(xyz, blh);

    sv_num = 0;
 
    for (i = 0; i < obs_c->sv_num; ++i)
    {

        fp64 azel[2]    = { 0 };
        fp64 rs[3]      = { 0 };
        fp64 vs[3]      = { 0 };
        fp64 sat_clk[2] = { 0 };
        fp64 iono_value = 0;
        fp64 iono_var   = 0;
        fp64 trop_value = 0;
        fp64 trop_var   = 0;
        fp64 e[3]       = { 0 };
        fp64 r          = 0;
        obs_sv_t *obs   = &obs_c->obs[i];
        int32_t j;

        if(!get_rs_clk(obs->sys_id, obs->sv_id, rs, vs, sat_clk, sat_info))
        {
            continue;
        }
        earth_rotate_corr(sat_info, obs->sv_id, obs->sys_id, &obs_c->rcv_info);
        r = geodist(rs, xyz, e);
        satazel(blh, e, obs->azel);
        iono_value = broadcast_iono_delay(obs_c->time, blh, obs, sat_info);
        iono_var   = SQR(iono_var * ERR_BRDCI);
        trop_value = mops_tropo_delay(blh[0], blh[2], obs->azel[1], (int32_t)time2doy(obs_c->time));

        if (sv_num != 0)
        {
            matrix_extend_row(H, 2);
            matrix_extend_row(&R_tmp, 2);
            matrix_extend_row(v, 2);
        }

        for (j = 0; j < est_num; ++j)
        {
            if (j < POS_PARAM_NUM)
            {
                H->element[sv_num * 2][j] = -e[j]; // for position param
                continue;
            }
            else if ((j >= POS_PARAM_NUM) && (j < (POS_PARAM_NUM + VEL_PARAM_NUM)))
            {
                H->element[sv_num * 2 + 1][j] = -e[j - POS_PARAM_NUM]; // for velosity param
                continue;
            }
            else if (j >= (POS_PARAM_NUM + VEL_PARAM_NUM) && (j < (POS_PARAM_NUM + VEL_PARAM_NUM + DTR_PARAM_NUM)))
            {
                H->element[sv_num * 2][j] = 1; // for clock bias
            }
            else
            {
                H->element[sv_num * 2 + 1][j] = 1; // for clock drift bias
            }
        }

        // for psudorange
        v->element[sv_num * 2][0]    = obs->P[0] - (r + dtr - CLIGHT * sat_clk[0]+ iono_value + trop_value); 
        fp64 psdrnge_var = get_psdrnge_var(obs->azel[1], obs->sys_id);
        R_tmp.element[sv_num * 2][0] = sat_info->gps_sat[obs->sv_id - 1].pos_var + iono_var + trop_var + psdrnge_var;

        /* for doppler */
        fp64 sv[3] = { 0 };
        for (uint32_t k = 0; k < 3; k++) sv[k] = vs[k] - vel[k];
        fp64 range_rate = dot(sv, e, 3) + OMGE / CLIGHT * (vs[2] * xyz[0] + rs[1] * vel[0] - vs[0] * xyz[1] - rs[0] * vel[1]);
        v->element[sv_num * 2 + 1][0]    = obs->D[0] * lam - (range_rate + dft - CLIGHT * sat_clk[1]);
        R_tmp.element[sv_num * 2 + 1][0] = SQR(0.5);
        sv_num++;
    }

    matrix_log(R_tmp, &loger, "R_TEM");
    matrix_init(R, R_tmp.row, R_tmp.row);
    for (uint32_t i = 0; i < R->col; ++i)
    {
        R->element[i][i] = R_tmp.element[i][0];
    }
    matrix_free(&R_tmp);

    return sv_num;
}

RETURN_STATUS LSQ(matrix_t *H, matrix_t *R, matrix_t *v, matrix_t  *dx, matrix_t *P)
{
    matrix_t HT;
    matrix_t HTR;
    matrix_t HTRH;
    matrix_t HTRv;
    
    matrix_init(&HT,   H->col,  H->row);
    matrix_init(&HTR,  HT.row,  R->col);
    matrix_init(&HTRH, HTR.row, H->col);
    matrix_init(&HTRv, HTR.row, v->col);

    matrix_trs(H,    &HT);
    matrix_mlt(&HT,  R, &HTR);
    matrix_mlt(&HTR, H, &HTRH);
    matrix_mlt(&HTR, v, &HTRv);
    
    matrix_log(HTR, &loger, "HTR");
    matrix_log(HTRv, &loger, "HTRv");

    if (matrix_inv(&HTRH, P))
    {
        matrix_mlt(P, &HTRv, dx);

        return RET_SUCCESS;
    }
    else
    {
        return RET_FAIL;
    }
}

static RETURN_STATUS spp_proc(obs_epoch_t *obs_c, sat_info_t *sat_info, spp_sol_t *spp_sol)
{
    int32_t i                   = 0;
    int32_t iter_num            = 0;
    int32_t act_obs_num         = 0;
    int32_t sv_num              = 0;
    static uint8_t spp_init     = false;

    matrix_t H, R,  v, dx, P;
    uint32_t est_num;
    ESTIMATE_PARAM_NUM(est_num);
    matrix_init(&dx, est_num, 1);
    matrix_init(&P, est_num, est_num);

    fp64 *appro_param = (fp64*)malloc(sizeof(fp64) * est_num);
    memset(appro_param, 0, sizeof(fp64) * est_num);

    if (!spp_init)
    {
        for (i = 0; i < 3; ++i)
        {
            appro_param[i] = obs_c->rcv_info.appro_pos[i];
            appro_param[i + POS_PARAM_NUM + DTR_PARAM_NUM] = 0;
        }
    }
    else
    {
        for (i = 0; i < 3; ++i)
        {
            appro_param[i] = spp_sol->pos[i];
            appro_param[i + POS_PARAM_NUM + DTR_PARAM_NUM] = spp_sol->vel[i];
        }
    }


    for (iter_num = 0; iter_num < 20; ++iter_num)
    {
        sv_num = construct_H_R_V_matrix(obs_c, sat_info, spp_sol, &H, &R, &v, spp_init, appro_param);

        matrix_log(H, &loger, "H");
        matrix_log(R, &loger, "R");
        matrix_log(v, &loger, "V");
        
        if (sv_num > 4)
        {
            if (LSQ(&H, &R, &v, &dx, &P))
            {
                for (uint32_t k = 0; k < est_num; ++k)
                {
                    appro_param[k] += dx.element[k][0];
                }
            }
        }
        
        if (fabs(dx.element[0][0]) < 0.1)
        {
            break;
        }
        //if (norm(dx, 3) < 1e-4)
        //{
        //    // TODO: ouput result
        //    break;
        //}
    }
    free(appro_param);
    spp_init = true;

    return RET_SUCCESS;
}

static bool check_start_end_time(const opt_file_t *opt_file, const fp64 *cur_ep)
{
    fp64 cur_time;
    fp64 start_time;
    fp64 end_time;
    fp64 start_ep[6];
    fp64 end_ep[6];

    for (uint8_t i = 0; i < 3; ++i)
    {
        start_ep[i]     = cur_ep[i];
        start_ep[i + 3] = opt_file->start_time[i];
        end_ep[i]       = cur_ep[i];
        end_ep[i + 3]   = opt_file->start_time[i];
    }

    start_time = epoch2time(start_ep);
    end_time   = epoch2time(end_ep);
    cur_time   = epoch2time(cur_ep);

    if (cur_time >= start_time && cur_time <= end_time)
    {
        return true;
    }
    else
    {
        return false;
    }
    
}

RETURN_STATUS proc(opt_file_t *opt_file)
{
    RETURN_STATUS ret_status       = false;
    uint8_t       is_open_obs_file = false;    // false: obs file has been opened; true: has not been opened.
    uint8_t       is_open_nav_file = false;
    uint8_t       is_run           = true;
    uint8_t       is_first_run     = true;
    spp_sol_t     spp_sol          = {0};

    while (is_run)
    {
        printf("is runing\n");
        sat_info_t sat_info;
        init_sat_info(&sat_info);

        /* read rinex obs file */
        load_curr_rinex_obs(opt_file, &obs_c, &is_open_obs_file, &is_run);

        if (!check_start_end_time(opt_file, obs_c.ep))
        {
            continue;
        }

        if (is_first_run)
        {
            /* read rinex nav file */
            read_rinex_nav_data(opt_file->nav_file, &sat_info.sys_ion_cor, &eph, &is_open_nav_file);
            is_first_run = false;
        }

        get_sv_pos_clk(&obs_c, &eph, &sat_info);
        
        spp_proc(&obs_c, &sat_info, &spp_sol);
    }
    
    return RET_SUCCESS;
}

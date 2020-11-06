#pragma once
#include "spp_sol.h"
#include "io.h"
#define BROADCAST_EPH_THRESHOLD	(7200)				//unit: sec
#define GPS_GM					(3.986004418E14)    // earth-gravitational constant reference to RTKLIB
#define OMGE					(7.2921151467E-5)   //earth angular velocity (IS-GPS) (rad/s)

/* global variable */
FILE        *obs_fp_ptr;
extern log_t loger;
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
		fp64	backward_time = time - VERY_BIG_NUM;
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
	u	  = atan2(sqrt(1.0 - eph_sat->E * eph_sat->E) * sin(E), cos(E) - eph_sat->E) + eph_sat->Omega;
	cos2u = cos(2 * u);
	sin2u = sin(2 * u);

	u	  += eph_sat->CuC * cos2u + eph_sat->CuS * sin2u;
	r	   = eph_sat->rootA * eph_sat->rootA * (1 - eph_sat->E * cos(E)) + eph_sat->CrC * cos2u + eph_sat->CrS * sin2u;
	eph_i  = eph_sat->I0 + eph_sat->CiC * cos2u + eph_sat->CiS * sin2u + eph_sat->Idot * tk;
	x	   = r * cos(u);
	y	   = r * sin(u);
	omg_tk = eph_sat->Omega0 + (eph_sat->OmegaDot - OMGE) * tk - OMGE * eph_sat->Toe;
 
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

	for (k = 0; k < obs_c->obs_num; ++k)
	{
		if (eph_sat->sv_id == obs_c->obs[k].sv_id && eph_sat->sys_id == obs_c->obs[k].sys_id)
		{
			break;
		}
	}
	if (k == obs_c->obs_num)
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
RETURN_STATUS get_sv_pos_clk(obs_epoch_t *obs_c, eph_t *eph, sat_info_t *sat_info)
{
	int32_t i = 0;
	for (i = 0; i < obs_c->obs_num; ++i)
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
		time_c  = obs_c->time - obs_c->obs[i].P[0] / CLIGHT;
        get_sat_pos_broadcast_eph(&eph_sat, &sat_pos, &sat_clk, time_c, &var);
		for (int32_t j = 0; j < 3; ++j)
		{
			sat_info->gps_sat[obs_c->obs[i].sv_id - 1].satpos[j] = sat_pos[j];
		}
		sat_info->gps_sat[obs_c->obs[i].sv_id - 1].satclk[0] = sat_clk;

		/* calculate satellite velocity and clock drift */
		time_s  = time_c - 1E-3;
		sat_clk = get_sv_clk_broadcast_eph(obs_c, &eph_sat, sat_info);
		get_sat_pos_broadcast_eph(&eph_sat, &sat_pos, &sat_clk, time_s, &var_s);
		for (int32_t j = 0; j < 3; ++j)
		{
			sat_info->gps_sat[obs_c->obs[i].sv_id - 1].satvel[j] = (sat_info->gps_sat[obs_c->obs[i].sv_id - 1].satpos[j] - sat_pos[j]) / 1E-3;
		}
		sat_info->gps_sat[obs_c->obs[i].sv_id - 1].satclk[1] = (sat_info->gps_sat[obs_c->obs[i].sv_id - 1].satclk[0] - sat_clk) / 1E-3;
	}
	
	return RET_SUCCESS;
}
static void init_sat_info(sat_info_t *sat_info)
{
	int32_t i;
	memset(sat_info, 0, sizeof(sat_info_t));
	for (i = 0; i < MAXGPSNUM; ++i)
	{
		sat_info->gps_sat[i].sys_id = SYS_GPS;
		sat_info->gps_sat[i].sv_id  = i + 1;
	}
}
RETURN_STATUS spp_proc(obs_epoch_t* obs_c, sat_info_t* sat_info)
{
	
	return RET_SUCCESS;
}
RETURN_STATUS proc(opt_file_t *opt_file)
{
	obs_epoch_t   obs_c;
	eph_t         eph;
	sys_ion_cor   ion_cor;
	RETURN_STATUS ret_status       = false;
	uint8_t		  is_open_obs_file = false;	// false: obs file has been opened; true: has not been opened.
	uint8_t		  is_open_nav_file = false;
	uint8_t		  is_run           = true;
	uint8_t		  is_fist_run	   = true;

	while (is_run)
	{
		sat_info_t sat_info;
		init_sat_info(&sat_info);

		/* read rinex obs file */
		load_curr_rinex_obs(opt_file->obs_file, &obs_c, &is_open_obs_file, &is_run);
		if (is_fist_run)
		{
			/* read rinex nav file*/
			read_rinex_nav_data(opt_file->nav_file, &ion_cor, &eph, &is_open_nav_file);
			is_fist_run = false;
		}
		get_sv_pos_clk(&obs_c, &eph, &sat_info);

		spp_proc(&obs_c, &sat_info);
	}
	
	return RET_SUCCESS;
}
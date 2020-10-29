#pragma once
#include "spp_sol.h"
#include "io.h"
#define BROADCAST_EPH_THRESHOLD  (7200) //unit: sec
/* global variable */
FILE        *obs_fp_ptr;
extern log_t loger;
eph_sat_t sel_broadcast_eph(fp64 time, int32_t sys_id, int32_t sat_id, eph_t *eph)
{
	eph_sat_t eph_sat = { 0 };
	if (sys_id & SYS_GPS)
	{
		int32_t i;
		for (i = 0; i < MAXGPSNUM; ++i)
		{
			if (sat_id == eph->gps_eph[i][0].sv_id)
			{
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
		}
	}
	return eph_sat;
}
RETURN_STATUS get_sat_pv_broadcast_eph(obs_epoch_t *obs_c, eph_sat_t *eph_sat, sat_info_t *sat_info)
{
	
}
RETURN_STATUS get_broadcast_eph_sv_clk(obs_epoch_t *obs_c, eph_sat_t *eph_sat, sat_info_t *sat_info)
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
	for (k = 0; k < obs_c->obs_num; ++k)
	{
		if (eph_sat->sv_id == obs_c->obs[k].sv_id && eph_sat->sys_id == obs_c->obs[k].sys_id)
		{
			break;
		}
	}
	if (k == obs_c->obs_num)
	{
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
		return RET_FAIL;
	}
	for (k = 0; k < 2; ++k)
	{
		delta_t -= eph_sat->sv_clk[0] + eph_sat->sv_clk[1] * delta_t + eph_sat->sv_clk[2] * delta_t * delta_t;
	}

	sat_info->gps_sat[m].satclk = eph_sat->sv_clk[0] + eph_sat->sv_clk[1] * delta_t + eph_sat->sv_clk[2] * delta_t * delta_t;

	return RET_SUCCESS;
	
}
RETURN_STATUS get_sv_pos_clk(obs_epoch_t *obs_c, eph_t *eph, sat_info_t *sat_info)
{
	int32_t i = 0;
	for (i = 0; i < obs_c->obs_num; ++i)
	{
		eph_sat_t eph_sat;
        eph_sat = sel_broadcast_eph(obs_c->time, obs_c->obs[i].sys_id, obs_c->obs[i].sv_id, eph);
		if (eph_sat.time[0] == 0)
		{
			return false;
		}
        get_broadcast_eph_sv_clk(obs_c, &eph_sat, sat_info);
        get_sat_pv_broadcast_eph(obs_c, &eph_sat, sat_info);
		
	}
	

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
RETURN_STATUS spp_proc(opt_file_t *opt_file)
{
	obs_epoch_t   obs_c;
	eph_t     eph;
	RETURN_STATUS ret_status       = false;
	uint8_t		  is_open_obs_file = false;	// false: obs file has been opened; true: has not been opened.
	uint8_t		  is_open_nav_file = false;
	uint8_t		  is_run           = true;
	uint8_t		  is_fist_run	   = true;
	while (is_run)
	{
		sat_info_t sat_info;
		init_sat_info(&sat_info);

		load_curr_rinex_obs(opt_file->obs_file, &obs_c, &is_open_obs_file, &is_run);
		if (is_fist_run)
		{
			read_rinex_nav_data(opt_file->nav_file, &eph, &is_open_nav_file);
			is_fist_run = false;
		}
		get_sv_pos_clk(&obs_c, &eph, &sat_info);
	}
	

	return RET_SUCCESS;
}
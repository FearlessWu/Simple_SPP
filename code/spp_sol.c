#pragma once
#include "spp_sol.h"
#include "io.h"

/* global variable */
FILE        *obs_fp_ptr;
extern log_t loger;
eph_sat_t *sel_broadcast_eph(fp64 time, int32_t sys_id, int32_t sat_id, eph_t* eph)
{
	
}
RETURN_STATUS get_sat_pv_broadcast_eph(obs_epoch_t *obs_c, eph_sat_t *eph_sat, sat_info_t *sat_info)
{
	
}
RETURN_STATUS get_broadcast_eph_sv_clk(obs_epoch_t *obs_c, eph_sat_t *eph_sat, sat_info_t *sat_info)
{

}
RETURN_STATUS get_sv_pos_clk(obs_epoch_t *obs_c, eph_t *eph, sat_info_t *sat_info)
{
	int32_t i = 0;
	for (i = 0; i < obs_c->obs_num; ++i)
	{
		eph_sat_t *eph_sat;
		eph_sat = sel_broadcast_eph(obs_c->time, obs_c->obs[i].sys_id, obs_c->obs[i].sv_id, eph);
        get_broadcast_eph_sv_clk(obs_c, eph_sat, sat_info);
        get_sat_pv_broadcast_eph(obs_c, eph_sat, sat_info);
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
		sat_info_t sat_info = { 0 };
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
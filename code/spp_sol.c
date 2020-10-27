#pragma once
#include "spp_sol.h"
#include "io.h"

/* global variable */
FILE        *obs_fp_ptr;
extern log_t loger;

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
		load_curr_rinex_obs(opt_file->obs_file, &obs_c, &is_open_obs_file, &is_run);
		if (is_fist_run)
		{
			read_rinex_nav_data(opt_file->nav_file, &eph, &is_open_nav_file);
			is_fist_run = false;
		}
	}
	

	return RET_SUCCESS;
}
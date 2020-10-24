#pragma once
#include "spp_sol.h"
#include "io.h"

log_t log = { 0 }; // global variable

RETURN_STATUS main(int32_t args, char *argv[])
{
    /* creat log file */
    open_log_file();

    /* read option file */
    opt_file_t opt_file = { 0 };
    RETURN_STATUS read_status = RET_FAIL;
    read_status = read_option_file(&opt_file, args, argv[1]);
    if (!read_status)
    {
        close_log_file();
        return RET_FAIL;
    }

    obs_epoch_t obs_c = { 0 };  // current epoch obs 
    eph_t eph = { 0 };          // all eph

    if (!load_current_obs(opt_file.obs_file, &obs_c))
    {
        // TODO: do something when load obs fail.
    }

    /* please begin your show here! */
    if (!load_broadcast_eph(opt_file.nav_file, &eph))
    {
        // TODO: do something when load broadcast ephemris fail.
    }

    close_log_file();

    return RET_SUCCESS;
}
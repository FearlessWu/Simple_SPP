#pragma once
#include "spp_sol.h"
#include "io.h"

log_t log = { 0 }; // global variable

RETURN_STATUS main(int32_t args, char *argv[])
{
    /* creat log file */
    open_log_file();

    /* read option file */
    opt_file_t    opt_file   = { 0 };
    RETURN_STATUS ret_status = RET_FAIL;
    ret_status = read_option_file(&opt_file, args, argv[1]);
    if (!ret_status)
    {   
        close_log_file();
    
        return RET_FAIL;
    }
    
    obs_epoch_t obs_c = { 0 };  // current epoch obs 
    eph_t       eph   = { 0 };  // all eph
    
    /* please begin your show here! */
    if (!load_broadcast_eph(opt_file.nav_file, &eph))
    {
        // TODO: do something when load broadcast ephemris fail.
    }
    
    ret_status = spp_proc(&opt_file);
    
    close_log_file();
    
    return RET_SUCCESS;
}
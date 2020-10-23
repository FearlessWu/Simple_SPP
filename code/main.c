#pragma once
#include "spp_sol.h"
#include "io.h"

RETURN_STATUS main(int32_t args, char* argv[])
{
    
    
    /* creat log file */
    log_t log = { 0 };
    open_log_file(&log);

    /* read option file */
    opt_file_t opt_file = { 0 };
    RETURN_STATUS read_status = RET_FAIL;

    switch (args)
    {
    case 1:
        read_status = read_default_opt_file(&opt_file);
        break;
    case 2:
        read_status = read_opt_file(&opt_file, argv[1]);
        break;
    default:
        return RET_FAIL;
    }
    if (!read_status)
    {
        close_log_file(&log);
        return RET_FAIL;
    }

    close_log_file(&log);

    return RET_SUCCESS;
}
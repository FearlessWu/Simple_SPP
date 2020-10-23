#pragma once
#include "io.h"
#include "spp_sol.h"

RETURN_STATUS read_opt_file(opt_file_t  *opt_file, char* opt_path)
{
    return RET_SUCCESS;
}

RETURN_STATUS read_default_opt_file(opt_file_t *opt_file)
{
    return RET_SUCCESS;
}
void open_log_file(log_t *log)
{
    if (!(log->log_file = fopen("..//log.txt","w")))
    {
        log->is_open = false;
        printf("log file open fail!\n");
    }
    else
    {
        log->is_open = true;
        printf("log file open success!\n");
    }
    
}

void close_log_file(log_t *log)
{
    if (log->is_open)
    {
        fclose(log->log_file);

    }
}

void record_to_log(log_t *log, error_level_t err_level, error_code_t err_code)
{
    if (!log->is_open)
    {
        return;
    }

    /* record error level */
    switch (err_level)
    {
    case WARNING:
        fprintf(log->log_file, "WARNING : ");
        fflush(log->log_file);
        break;
    case FATAL:
        fprintf(log->log_file, "FATAL ERR: ");
        fflush(log->log_file);
        break;
    default:
        break; // do not except jump into here!   
    }

    /* record error detail */
    switch (err_code)
    {
    case NO_OBS_FILE:
        fprintf(log->log_file, "ERROR CODE = %2d, NO OBS FILE.\n", err_code);
        fflush(log->log_file);
        break;
    case NO_NAV_FILE:
        fprintf(log->log_file, "ERROR CODE = %2d, NO NAV FILE.\n", err_code);
        fflush(log->log_file);
        break;
    default:
        break; // do not except jump into here!  

    }

}
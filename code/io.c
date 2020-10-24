#pragma once
#include "io.h"
#include "spp_sol.h"
#include <string.h>

extern log_t log;

static RETURN_STATUS read_opt_body(opt_file_t *opt_file, FILE *fp)
{
    char buff[1024];
    const int32_t buff_size = 1024;
    while ((fgets(buff, buff_size, fp)) != NULL)

    {
        if (strstr(buff, "obs_file_path") != NULL)
        {
            strncpy(opt_file->obs_file, buff + 22, 255);
        }

		if (strstr(buff, "nav_file_path") != NULL)
		{
            strncpy(opt_file->nav_file, buff + 22, 255);
		}

    }

    return RET_SUCCESS;
}
RETURN_STATUS read_opt_file(opt_file_t  *opt_file, char *opt_path)
{
    FILE *opt_fp;
    if (!(opt_fp = fopen(opt_path, "r"))) 
    {
        if (log.is_open)
        {
            record_to_log(FATAL, CANT_READ_OPT_FILE);
        }

        return RET_FAIL;
    }
    read_opt_body(opt_file, opt_fp);
    
    fclose(opt_fp);

    return RET_SUCCESS;
}

RETURN_STATUS read_default_opt_file(opt_file_t *opt_file)
{
	FILE* opt_fp;
	if (!(opt_fp = fopen("..//Simple_SPP//SPP.opt", "r")))
	{
		if (log.is_open)
		{
			record_to_log(FATAL, CANT_READ_OPT_FILE);
		}

		return RET_FAIL;
	}
	read_opt_body(opt_file, opt_fp);

	fclose(opt_fp);

    return RET_SUCCESS;
}
void open_log_file()
{
    if (!(log.log_file = fopen("..//Simple_SPP//log.txt","w")))
    {
        log.is_open = false;
        printf("log file open fail!\n");
    }
    else
    {
        log.is_open = true;
        printf("log file open success!\n");
    }
    
}

void close_log_file()
{
    if (log.is_open)
    {
        fclose(log.log_file);
    }
}

void record_to_log(error_level_t err_level, error_code_t err_code)
{
    if (!log.is_open)
    {
        return;
    }

    /* record error level */
    switch (err_level)
    {
    case WARNING:
        fprintf(log.log_file, "WARNING : ");
        fflush(log.log_file);
        break;
    case FATAL:
        fprintf(log.log_file, "FATAL ERR: ");
        fflush(log.log_file);
        break;
    default:
        break; // do not except jump into here!   
    }

    /* record error detail */
    switch (err_code)
    {
    case NO_OBS_FILE:
        fprintf(log.log_file, "ERROR CODE = %2d, NO OBS FILE.\n", err_code);
        fflush(log.log_file);
        break;
    case NO_NAV_FILE:
        fprintf(log.log_file, "ERROR CODE = %2d, NO NAV FILE.\n", err_code);
        fflush(log.log_file);
        break;
    case CANT_READ_OPT_FILE:
		fprintf(log.log_file, "ERROR CODE = %2d, CANT_READ_OPT_FILE\n", err_code);
		fflush(log.log_file);
		break;
    default:
        break; // do not except jump into here!  
    }

}

RETURN_STATUS read_option_file(opt_file_t *opt_file, int32_t args, char* opt_file_path)
{
    RETURN_STATUS read_status = RET_FAIL;

	switch (args)
	{
	case 1:
		read_status = read_default_opt_file(opt_file);
		break;
	case 2:
		read_status = read_opt_file(opt_file, opt_file_path);
		break;
	default:
		return RET_FAIL;
	}

    return read_status;
}

RETURN_STATUS load_current_obs(char* obs_file_path, obs_epoch_t* obs_c)
{
	// TODO£ºread obs file 
	return RET_SUCCESS;
}

RETURN_STATUS load_broadcast_eph(char* nav_file_path, eph_t* eph)
{
	// TODO: read nav file
	return RET_SUCCESS;
}
#pragma once
#include "io.h"
#include "spp_sol.h"
#include <string.h>

/* global variable */
extern log_t log;
extern FILE* obs_fp_ptr;

/* remove the newline symbol */
static void remove_newline_symbol(char* in)
{
	char* tmp = NULL;
	if ((tmp = strstr(in, "\n")))
	{
		*tmp = '\0';
	}
}

/* add stop char */
static RETURN_STATUS add_stop_char(char *in, int32_t pos)
{
    if (in == NULL)
    {
        return RET_FAIL;
    }

    in[pos] = '\0';

    return RET_SUCCESS;
}

static RETURN_STATUS read_opt_body(opt_file_t *opt_file, FILE *fp)
{
    char buff[1024];
    const int32_t buff_size = 1024;

    while ((fgets(buff, buff_size, fp)) != NULL)
    {
        if (strstr(buff, "obs_file_path") != NULL)
        {
            strncpy(opt_file->obs_file, buff + 22, 255);
            remove_newline_symbol(opt_file->obs_file);
        }

		if (strstr(buff, "nav_file_path") != NULL)
		{
            strncpy(opt_file->nav_file, buff + 22, 255);
            remove_newline_symbol(opt_file->nav_file);
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

RETURN_STATUS read_option_file(opt_file_t *opt_file, int32_t args, char *opt_file_path)
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

static RETURN_STATUS read_rinex_obs_header(char *obs_file_path, rcv_info_t *rcv_info, uint8_t *is_open_obs_file)
{
	if (!(obs_fp_ptr = fopen(obs_file_path, "r")))
	{
		// TODO: do something when open obs file fail!
		return RET_FAIL;
	}
    *is_open_obs_file = true;

    char    buff[1024];
    char    sub_buff[1024];
    int32_t buff_size   = 1024;
    fp64    rnx_version = 0;

    /* get rinex version */
    if ((fgets(buff, buff_size, obs_fp_ptr)) != NULL)
    {
		if (strstr(buff, "RINEX VERSION / TYPE") != NULL)
		{
			strncpy(sub_buff, buff, 9);
            add_stop_char(sub_buff, 9);
            rnx_version = atof(sub_buff);
		}
    }
    else
    {
        return RET_FAIL;
    }

    if (rnx_version <= 2.99)
    {
        //TODO: NOT SUPPORT YET
        return RET_FAIL;
    }

	while ((fgets(buff, buff_size, obs_fp_ptr)) != NULL)
	{
		if (strstr(buff, "MARKER NAME") != NULL)
		{
            strncpy(rcv_info->sta_name, buff, 4);
            add_stop_char(rcv_info->sta_name, 4);

            continue;
		}

        if (strstr(buff, "END OF HEADER") != NULL)
        {
            return RET_SUCCESS;
        }

        if (strstr(buff, "ANT # / TYPE") != NULL)
        {
            strncpy(rcv_info->rcv_type, buff + 20, 7);
            add_stop_char(rcv_info->rcv_type, 7);

            continue;
        }

        if (strstr(buff, "APPROX POSITION XYZ") != NULL)
        {
            strncpy(sub_buff, buff, 14);
            add_stop_char(sub_buff, 14);
            rcv_info->appro_pos[0] = atof(sub_buff);

            strncpy(sub_buff, buff + 14, 14);
            add_stop_char(sub_buff, 14);
            rcv_info->appro_pos[1] = atof(sub_buff);

            strncpy(sub_buff, buff + 28, 14);
            add_stop_char(sub_buff, 14);
            rcv_info->appro_pos[2] = atof(sub_buff);

            continue;
        }

        if (strstr(buff, "ANTENNA: DELTA H/E/N") != NULL)
        {
            strncpy(sub_buff, buff, 14);
            add_stop_char(sub_buff, 14);
            rcv_info->atx_offset[0] = atof(sub_buff);

            strncpy(sub_buff, buff + 14, 14);
            add_stop_char(sub_buff, 14);
            rcv_info->atx_offset[1] = atof(sub_buff);

            strncpy(sub_buff, buff + 28, 14);
            add_stop_char(sub_buff, 14);
            rcv_info->atx_offset[2] = atof(sub_buff);

            continue;
        }
        
        if (strstr(buff, "SYS / # / OBS TYPES") != NULL)
        {
            char tpm[20][4];
            char sys[1];
            int32_t type_num = 0;
            int32_t row_num = 0;

            strncpy(sys, buff, 1);
            strncpy(sub_buff, buff + 3, 3);
            add_stop_char(sub_buff, 3);
            type_num = atoi(sub_buff);
            row_num = (type_num / 13) + 1;

            int32_t i = 0;
            if (row_num == 1)
            {
                for (i = 0; i < type_num; ++i)
                {
                    int32_t k = (i * 4);
                    strncpy(tpm[i], buff + 7 + k, 3);
                    add_stop_char(tpm[i], 3);
                }
            }
            else
            {
                int32_t j = 0;

                for (j = 0; j < row_num - 1; ++j)
                {
                    for (i = 0; i < 13; ++i)
                    {
                        int32_t k = (i * 4);
                        strncpy(tpm[j * 13 + i], buff + 7 + k, 3);
                        add_stop_char(tpm[j * 13 + i], 3);
                    }
                    fgets(buff, buff_size, obs_fp_ptr);
                }
                
                int32_t res_num = type_num - (row_num - 1) * 13;
                for (i = 0; i < res_num - 1; ++i)
                {
                    int32_t k = (i * 4);
                    strncpy(tpm[(row_num - 1) * 13 + i], buff + 7 + k, 3);
                    add_stop_char(tpm[(row_num - 1) * 13 + i], 3);
                }
            }
            
            switch (sys[0])
            {
            case 'G':
                for (i = 0; i < type_num; ++i)
                {
                    strcpy(rcv_info->gps_obs_type[i], tpm[i]);
                }
                break;
            case 'R':
                for (i = 0; i < type_num; ++i)
                {
                    strcpy(rcv_info->glo_obs_type[i], tpm[i]);
                }
                break;
            case 'E':
                for (i = 0; i < type_num; ++i)
                {
                    strcpy(rcv_info->gal_obs_type[i], tpm[i]);
                }
                break;
            case 'C':
                for (i = 0; i < type_num; ++i)
                {
                    strcpy(rcv_info->bds_obs_type[i], tpm[i]);
                }
                break;
            default:
                break;
            }

        }
	}

    return RET_FAIL;
}

static void read_rinex_obs_body(obs_epoch_t *obs, uint8_t *is_run)
{

}
RETURN_STATUS load_curr_rinex_obs(char *obs_file_path, obs_epoch_t *obs, uint8_t *is_open_obs_file, uint8_t *is_run)
{
    read_rinex_obs_header(obs_file_path, &obs->rcv_info, is_open_obs_file);
    read_rinex_obs_body(obs, is_run);
    
	return RET_SUCCESS;
}

RETURN_STATUS load_broadcast_eph(char* nav_file_path, eph_t* eph)
{
	// TODO: read nav file
	return RET_SUCCESS;
}

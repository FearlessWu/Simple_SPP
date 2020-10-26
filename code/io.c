#pragma once
#include "io.h"
#include "spp_sol.h"
#include <string.h>

/* global variable */
extern log_t loger;
extern FILE* obs_fp_ptr;
extern FILE* nav_fp_ptr;

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
        if (loger.is_open)
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
		if (loger.is_open)
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
    if (!(loger.log_file = fopen("..//Simple_SPP//log.txt","w")))
    {
        loger.is_open = false;
        printf("log file open fail!\n");
    }
    else
    {
        loger.is_open = true;
        printf("log file open success!\n");
    }
    
}

void close_log_file()
{
    if (loger.is_open)
    {
        fclose(loger.log_file);
    }
}

void record_to_log(error_level_t err_level, error_code_t err_code)
{
    if (!loger.is_open)
    {
        return;
    }

    /* record error level */
    switch (err_level)
    {
    case WARNING:
        fprintf(loger.log_file, "WARNING : ");
        fflush(loger.log_file);
        break;
    case FATAL:
        fprintf(loger.log_file, "FATAL ERR: ");
        fflush(loger.log_file);
        break;
    default:
        break; // do not except jump into here!   
    }

    /* record error detail */
    switch (err_code)
    {
    case NO_OBS_FILE:
        fprintf(loger.log_file, "ERROR CODE = %2d, NO OBS FILE.\n", err_code);
        fflush(loger.log_file);
        break;
    case NO_NAV_FILE:
        fprintf(loger.log_file, "ERROR CODE = %2d, NO NAV FILE.\n", err_code);
        fflush(loger.log_file);
        break;
    case CANT_READ_OPT_FILE:
		fprintf(loger.log_file, "ERROR CODE = %2d, CANT_READ_OPT_FILE\n", err_code);
		fflush(loger.log_file);
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

RETURN_STATUS read_rinex_nav_data(char* nav_file_path, eph_t* all_eph_info, uint8_t* is_open_nav_file)
{
    char string[90];
    char nav_header[64] = { "END OF HEADER" };
    eph_sat_t temp_eph;
    fp64 temp;
    int16_t  lines = 0;
    uint16_t i = 0;
    uint32_t svid;
    FILE* nav_fp_ptr;
    
    if (!(nav_fp_ptr = fopen(nav_file_path, "r")))
	{
		// TODO: debugout no nav file!
		return RET_FAIL;
	}
    *is_open_nav_file = true;

    memset(&all_eph_info, 0, sizeof(eph_t));
    svid =  0;
    while (fgets(string, 89, nav_fp_ptr))
    {
        if (strstr(string, nav_header, strlen(nav_header)))
        {
            break;
        }
    }
    while (fgets(string, 89, nav_fp_ptr))
    {
        
        if(string[0] == 'G')
        { 
            lines = 5;
            memset(&temp_eph, 0, sizeof(eph_sat_t));
            temp_eph.nav_valid = 1;
            temp_eph.sys_id = 0;
            if (sscanf(string + strlen('G'), "%d %d %d %d %d %d %d %lf %lf %lf",
                &temp_eph.sv_id, &temp_eph.time[0], &temp_eph.time[1], &temp_eph.time[2], &temp_eph.time[3], &temp_eph.time[4], &temp_eph.Toc,
                &temp_eph.sv_clk[0], &temp_eph.sv_clk[1], &temp_eph.sv_clk[2]) != 10)
            {
                temp_eph.nav_valid = 0;
                continue;
            }
            
            if (svid = temp_eph.sv_id)
            {
                i += 1;
            }
            else
            {
                svid = temp_eph.sv_id;
                i = 0;
            }

           while(lines && temp_eph.nav_valid)
            {
               fgets(string, 89, nav_fp_ptr);
               switch (6 - lines)
               {
                case 1:
                    if (sscanf(string, "%lf %lf %lf %lf",
                        &temp, &temp_eph.CrS, &temp_eph.DeltaN, &temp_eph.M0) != 4)
                    {
                        temp_eph.nav_valid = 0;
                    }
                    break;
                case 2:
                    if (sscanf(string, "%lf %lf %lf %lf",
                        &temp_eph.CuC, &temp_eph.E, &temp_eph.CuS, &temp_eph.rootA) != 4)
                    {
                        temp_eph.nav_valid = 0;
                    }
                    break;
                case 3:
                    if (sscanf(string, "%lf %lf %lf %lf",
                        &temp_eph.Toe, &temp_eph.CiC, &temp_eph.Omega0, &temp_eph.CiS) != 4)
                    {
                        temp_eph.nav_valid = 0;
                    }
                    break;
                case 4:
                    if (sscanf(string, "%lf %lf %lf %lf",
                        &temp_eph.I0, &temp_eph.CrC, &temp_eph.Omega, &temp_eph.OmegaDot) != 4)
                    {
                        temp_eph.nav_valid = 0;
                    }
                    break;
                case 5:
                    if (sscanf(string, "%lf %lf %lf %lf",
                        &temp_eph.Idot, &temp, &temp, &temp) != 4)
                    {
                        temp_eph.nav_valid = 0;
                    }
                    break;
                default:
                    break;
               }
               lines -= 1;

            }
           if(temp_eph.nav_valid)
               all_eph_info->gps_eph[temp_eph.sv_id - 1][i] = temp_eph;
        }
    }
   
    return RET_SUCCESS;
}

static RETURN_STATUS read_rinex_obs_header(char* obs_file_path, rcv_info_t* rcv_info, uint8_t* is_open_obs_file)
{
    if (*is_open_obs_file)
    {
        return RET_SUCCESS;
    }
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
static void find_obs_type_idx(int32_t *gps_type_idx, char **gps_type, obs_epoch_t *obs)
{
    int32_t i;
    int32_t count = 0;
    for (i = 0; i < 20; ++i)
    {
        if (!strcmp(obs->rcv_info.gps_obs_type[i], gps_type[0]))
        {
            gps_type_idx[0] = i + 1;
            count++;
            if (count == 2)
            {
                return;
            }
        }

        if (!strcmp(obs->rcv_info.gps_obs_type[i], gps_type[1]))
        {
            gps_type_idx[1] = i + 1;
            count++;
            if (count == 2)
            {
                return;
            }
        }
    }
}
static RETURN_STATUS read_rinex_obs_body(obs_epoch_t *obs, uint8_t *is_run)
{
    char    buff[1024];
    char    sub_buff[1024];
    int32_t buff_size = 1024;
    int32_t i;
    int32_t sv_num;
    char    *gps_type[2] = { "C1C\0","C2S\0" };
    int32_t gps_type_idx[2] = { 0 };

    find_obs_type_idx(gps_type_idx, gps_type, obs);

    while (fgets(buff, buff_size, obs_fp_ptr))
    {
        if (strstr(buff, ">") == NULL)
        {
            return RET_FAIL;
        }
        
        /* read epoch time*/
        sv_num = 0;
        strncpy(sub_buff, buff + 2, 4);
        add_stop_char(sub_buff, 4);
        obs->ep[0] = atof(sub_buff);
        for (i = 1; i < 5; ++i)
        {
            int8_t k = (i - 1) * 3;
            strncpy(sub_buff, buff + 7 + k, 2);
            add_stop_char(sub_buff, 2);
            obs->ep[i] = atof(sub_buff);
        }
        strncpy(sub_buff, buff + 18, 11);
        add_stop_char(sub_buff, 11);
        obs->ep[5] = atof(sub_buff);

        obs->time = epoch2time(obs->ep);

        strncpy(sub_buff, buff + 31, 1);
        add_stop_char(sub_buff, 1);
        obs->epoch_flag = atoi(sub_buff);

        strncpy(sub_buff, buff + 32, 3);
        add_stop_char(sub_buff, 3);
        sv_num = atoi(sub_buff);
#if 0
        if (!(obs->obs = (obs_sv_t *)malloc(sizeof(obs_sv_t) * sv_num)))
        {
            // TODO: do something informs developer
            return RET_FAIL;
        }
#endif
        obs->obs_num = 0;
        for (i = 0; i < sv_num; ++i)
        {
            if (fgets(buff, buff_size, obs_fp_ptr) != NULL)
            {
                strncpy(sub_buff, buff, 1);
                if (sub_buff[0] != 'G')
                {
                    continue;
                }
                obs->obs[obs->obs_num].sys_id = SYS_GPS;
                strncpy(sub_buff, buff + 1, 2);
                add_stop_char(sub_buff, 2);
                obs->obs[obs->obs_num].sv_id = atoi(sub_buff);

                int32_t j;
                for (j = 0; j < 2; ++j)
                {
                    /* pseudorange */
                    int32_t k = 3 + (gps_type_idx[j] - 1) * 16;
                    strncpy(sub_buff, buff + k, 14);
                    add_stop_char(sub_buff, 14);
                    obs->obs[obs->obs_num].P[j] = atof(sub_buff);
                    if (fabs(obs->obs[obs->obs_num].P[j]) > 0.001)
                    {
                        obs->obs[obs->obs_num].P_status[j] = USE;
                    }
                    else
                    {
                        obs->obs[obs->obs_num].P_status[j] = NOT_USE;
                    }
                    
                    /* phase */
                    k += 16;
                    strncpy(sub_buff, buff + k, 14);
                    add_stop_char(sub_buff, 14);
                    obs->obs[obs->obs_num].L[j] = atof(sub_buff);
                    if (fabs(obs->obs[obs->obs_num].L[j]) > 0.001)
                    {
                        obs->obs[obs->obs_num].L_status[j] = USE;
                    }
                    else
                    {
                        obs->obs[obs->obs_num].L_status[j] = NOT_USE;
                    }

                    /* LLI */
                    strncpy(sub_buff, buff + k +14, 1);
                    add_stop_char(sub_buff, 1);
                    obs->obs[obs->obs_num].LLI[j] = atoi(sub_buff);

                    /* signal strength */
                    k += 16;
                    strncpy(sub_buff, buff + k, 14);
                    add_stop_char(sub_buff, 14);
                    obs->obs[obs->obs_num].D[j] = atof(sub_buff);
                    if (fabs(obs->obs[obs->obs_num].D[j]) > 0.001)
                    {
                        obs->obs[obs->obs_num].D_status[j] = USE;
                    }
                    else
                    {
                        obs->obs[obs->obs_num].D_status[j] = NOT_USE;
                    }

                    /* signal strength */
                    k += 16;
                    strncpy(sub_buff, buff + k, 14);
                    add_stop_char(sub_buff, 14);
                    obs->obs[obs->obs_num].CN0[j] = atof(sub_buff);
                }
            }
            obs->obs_num++;
        }
        return RET_SUCCESS;
    }

    *is_run = false;
    return RET_FAIL;
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

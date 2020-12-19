
#include "io.h"
#include "spp_sol.h"

#define GPS_L1_SIGNAL_NUM   (35)
#define GPS_L2_SIGNAL_NUM   (39)
#define GPS_L5_SIGNAL_NUM   (12)

/* global variable */
extern log_t loger;
extern FILE* obs_fp_ptr;
extern FILE* nav_fp_ptr;
extern const char *error_message[];

 static const char *L1_signal_type[GPS_L1_SIGNAL_NUM] =
 {
   "C1C", "L1C", "D1C", "S1C", "C1S", "L1S", "D1S", "S1S",
   "C1L", "L1L", "D1L", "S1L", "C1X", "L1X", "D1X", "S1X",
   "C1P", "L1P", "D1P", "S1P", "C1W", "L1W", "D1W", "S1W",
   "C1Y", "L1Y", "D1Y", "S1Y", "C1M", "L1M", "D1M", "S1M",
          "L1N", "D1N", "S1N"
 };
 
 static const char *L2_signal_type[GPS_L2_SIGNAL_NUM] = 
 {
   "C2C", "L2C", "D2C", "S2C", "C2D", "L2D", "D2D", "S2D",
   "C2S", "L2S", "D2S", "S2S", "C2L", "L2L", "D2L", "S2L", 
   "C2X", "L2X", "D2X", "S2X", "C2P", "L2P", "D2P", "S2P",
   "C2W", "L2W", "D2W", "S2W", "C2Y", "L2Y", "D2Y", "S2Y",
   "C2M", "L2M", "D2M", "S2M",        "L2N", "D2N", "S2N"
 };

 static const char *L5_signal_type[GPS_L5_SIGNAL_NUM] =
 {
    "C5I", "L5I", "D5I", "S5I", "C5Q", "L5Q", "D5Q", "S5Q",
    "C5X", "L5X", "D5X", "S5X"
 };

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
    char sub_buff[512];

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

        if (strstr(buff, "freq_num") != NULL)
        {
            strncpy(sub_buff, buff + 22, 255);
            add_stop_char(sub_buff, 1);
            opt_file->freq_num = atoi(sub_buff);
        }

        if (strstr(buff, "freq_type") != NULL)
        {
            for (int32_t i = 0; i < opt_file->freq_num; ++i)
            {
                strncpy(opt_file->freq_type[i], buff + 22 + i * 4, 3);
                remove_newline_symbol(opt_file->freq_type[i]);
            }
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
            print_log(NULL, CANT_READ_OPT_FILE, error_message[CANT_READ_OPT_FILE]);
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
            print_log(NULL, CANT_READ_OPT_FILE, error_message[CANT_READ_OPT_FILE]);
        }

        return RET_FAIL;
    }
    read_opt_body(opt_file, opt_fp);

    fclose(opt_fp);

    return RET_SUCCESS;
}

void open_log_file()
{
    if (!(loger.log_fp = fopen("..//Simple_SPP//log.txt","w")))
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
        fclose(loger.log_fp);
    }
}

void print_log(fp64 *time, error_code_t err_code, const char *message)
{
    if (!loger.is_open)
    {
        return;
    }
    if (time == NULL)
    {
        fprintf(loger.log_fp, ",,,,,,,,,,,,Error code: %d, %s\n", err_code, message);
    }
    else
    {
        fprintf(loger.log_fp, "%9.2f,,,Error code: %d, %s\n", *time, err_code, message);
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

RETURN_STATUS read_rinex_nav_data(char *nav_file_path, sys_ion_cor_t  *all_ion_cor, eph_t *all_eph_info, uint8_t *is_open_nav_file)
{
    char string[90], p[20];
    char nav_header[64] = { "END OF HEADER" };
    eph_sat_t temp_eph;
    int16_t  lines = 0;
    uint32_t i = 0, j = 0;
    uint32_t svid;
    FILE* nav_fp_ptr;
    
    if (!(nav_fp_ptr = fopen(nav_file_path, "r")))
    {
        // TODO: debugout no nav file!
        return RET_FAIL;
    }
    *is_open_nav_file = true;

    memset(all_ion_cor, 0, sizeof(sys_ion_cor_t));
    memset(all_eph_info, 0, sizeof(eph_t));
    svid =  0;
    while (fgets(string, 89, nav_fp_ptr))
    {
        if (string[0] == 'G' && string[1] == 'P' && string[2] == 'S' && string[3] == 'A')
        {
            for (j = 0; j < 4; ++j)
            {
                int8_t k = j * 12;
                    strncpy(p, string + 5 + k, 12);
                    add_stop_char(p, 12);
                    all_ion_cor->gps_ino_cor.alph[j] = atof(p);
            }
        }

        if (string[0] == 'G' && string[1] == 'P' && string[2] == 'S' && string[3] == 'B')
        {
            for (j = 0; j < 4; ++j)
            {
                int8_t k = j * 12;
                strncpy(p, string + 5 + k, 12);
                add_stop_char(p, 12);
                all_ion_cor->gps_ino_cor.beta[j] = atof(p);
            }
        }

        if (strstr(string, nav_header))
        {
            break;
        }
    }
    while (fgets(string, 89, nav_fp_ptr))
    {
        
        if(string[0] == 'G')
        { 
            lines = 6;
            memset(&temp_eph, 0, sizeof(eph_sat_t));
            temp_eph.nav_valid = 1;
            temp_eph.sys_id = SYS_GPS;

            strncpy(p, string + 1, 2);
            add_stop_char(p, 2);
            temp_eph.sv_id = atoi(p);
            strncpy(p, string + 4, 4);
            add_stop_char(p, 4);
            temp_eph.time[0] = atoi(p);

            for (j = 0; j < 4; ++j)
            {   
                int8_t k = j * 3;
                strncpy(p, string + 9 + k, 2);
                add_stop_char(p, 2);
                temp_eph.time[ j + 1] = atoi(p);
            }

            strncpy(p, string + 21, 2);
            add_stop_char(p, 2);
            temp_eph.Toc = atoi(p);

            for (j = 0; j < 3; ++j)
            {
                int8_t k = j * 19;
                strncpy(p, string + 23 + k, 19);
                add_stop_char(p, 19);
                temp_eph.sv_clk[j] = atof(p);
            }
                                    
            if (svid == temp_eph.sv_id)
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
               switch (7 - lines)
               {
                case 1:
                   
                    strncpy(p, string + 4 + 19 * 1, 19);
                    add_stop_char(p, 19);
                    temp_eph.CrS = atof(p);
                    strncpy(p, string + 4 + 19 * 2, 19);
                    add_stop_char(p, 19);
                    temp_eph.DeltaN = atof(p);
                    strncpy(p, string + 4 + 19 * 3, 19);
                    add_stop_char(p, 19);
                    temp_eph.M0 = atof(p);
                    
                    break;
                case 2:

                    strncpy(p, string + 4 + 19 * 0, 19);
                    add_stop_char(p, 19);
                    temp_eph.CuC = atof(p);
                    strncpy(p, string + 4 + 19 * 1, 19);
                    add_stop_char(p, 19);
                    temp_eph.E = atof(p);
                    strncpy(p, string + 4 + 19 * 2, 19);
                    add_stop_char(p, 19);
                    temp_eph.CuS = atof(p);
                    strncpy(p, string + 4 + 19 * 3, 19);
                    add_stop_char(p, 19);
                    temp_eph.rootA = atof(p);

                    break;
                case 3:

                    strncpy(p, string + 4 + 19 * 0, 19);
                    add_stop_char(p, 19);
                    temp_eph.Toe = atof(p);
                    strncpy(p, string + 4 + 19 * 1, 19);
                    add_stop_char(p, 19);
                    temp_eph.CiC = atof(p);
                    strncpy(p, string + 4 + 19 * 2, 19);
                    add_stop_char(p, 19);
                    temp_eph.Omega0 = atof(p);
                    strncpy(p, string + 4 + 19 * 3, 19);
                    add_stop_char(p, 19);
                    temp_eph.CiS = atof(p);

                    break;
                case 4:

                    strncpy(p, string + 4 + 19 * 0, 19);
                    add_stop_char(p, 19);
                    temp_eph.I0 = atof(p);
                    strncpy(p, string + 4 + 19 * 1, 19);
                    add_stop_char(p, 19);
                    temp_eph.CrC = atof(p);
                    strncpy(p, string + 4 + 19 * 2, 19);
                    add_stop_char(p, 19);
                    temp_eph.Omega = atof(p);
                    strncpy(p, string + 4 + 19 * 3, 19);
                    add_stop_char(p, 19);
                    temp_eph.OmegaDot = atof(p);                    

                    break;
                case 5:

                    strncpy(p, string + 4 + 19 * 0, 19);
                    add_stop_char(p, 19);
                    temp_eph.Idot = atof(p);

                    break;
                case 6:

                    strncpy(p, string + 4 + 19 * 0, 19);
                    temp_eph.sv_acc = atof(p);
                    strncpy(p, string + 4 + 19 * 1, 19);
                    temp_eph.sv_hea = atof(p);
                    strncpy(p, string + 4 + 19 * 2, 19);
                    add_stop_char(p, 19);
                    temp_eph.Tgd = atof(p);

                    break;
                default:
                    break;
               }
               lines -= 1;

            }

           if (temp_eph.nav_valid)
           {
               if (svid >= 1 && i >= 0)
                   memcpy(&all_eph_info->gps_eph[svid - 1][i], &temp_eph, sizeof(temp_eph));
           }            
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

static void find_obs_type_idx(int32_t *gps_type_idx, char **gps_type, obs_epoch_t *obs, uint8_t freq_num)
{
    int32_t i;
    int32_t count = 0;
    for (i = 0; i < 20; ++i)
    {
        if (!strcmp(obs->rcv_info.gps_obs_type[i], gps_type[0]))
        {
            gps_type_idx[0] = i + 1;
            count++;
            if (count == freq_num)
            {
                return;
            }
        }

        if (!strcmp(obs->rcv_info.gps_obs_type[i], gps_type[1]))
        {
            gps_type_idx[1] = i + 1;
            count++;
            if (count == freq_num)
            {
                return;
            }
        }
    }
}

static void get_lam(char **signal_type, fp64 *lam, uint8_t freq_num)
{
    for (uint8_t j = 0; j < freq_num; ++j)
    {
        for (uint8_t i = 0; i < GPS_L1_SIGNAL_NUM; ++i)
        {
            if (!(strcmp(signal_type[j], L1_signal_type[i])))
            {
                lam[j] = FREQ1;

                break;
            }

            if (!(strcmp(signal_type[j], L2_signal_type[i])))
            {
                lam[j] = FREQ2;
                
                break;
            }
        }
    }
    
}

static RETURN_STATUS read_rinex_obs_body(opt_file_t *opt_file, obs_epoch_t *obs, uint8_t *is_run)
{
    char    buff[1024];
    char    sub_buff[1024];
    int32_t buff_size = 1024;
    int32_t i;
    int32_t sv_num;
    fp64    lam[2] = {0.0};
    char    *gps_type[2]    = { opt_file->freq_type[1], opt_file->freq_type[0] };
    int32_t gps_type_idx[2] = { 0 };

    find_obs_type_idx(gps_type_idx, gps_type, obs, opt_file->freq_num);
    get_lam(gps_type, lam, opt_file->freq_num);

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
        obs->sv_num = 0;
        for (i = 0; i < sv_num; ++i)
        {
            if (fgets(buff, buff_size, obs_fp_ptr) != NULL)
            {
                strncpy(sub_buff, buff, 1);
                if (sub_buff[0] != 'G')
                {
                    continue;
                }
                obs->obs[obs->sv_num].sys_id = SYS_GPS;
                strncpy(sub_buff, buff + 1, 2);
                add_stop_char(sub_buff, 2);
                obs->obs[obs->sv_num].sv_id = atoi(sub_buff);

                int32_t j;
                for (j = 0; j < opt_file->freq_num; ++j)
                {
                    /* pseudorange */
                    int32_t k = 3 + (gps_type_idx[j] - 1) * 16;
                    strncpy(sub_buff, buff + k, 14);
                    add_stop_char(sub_buff, 14);
                    obs->obs[obs->sv_num].P[j] = atof(sub_buff);
                    if (fabs(obs->obs[obs->sv_num].P[j]) > 0.001)
                    {
                        obs->obs[obs->sv_num].P_status[j] = USE;
                    }
                    else
                    {
                        obs->obs[obs->sv_num].P_status[j] = NOT_USE;
                    }
                    
                    /* phase */
                    k += 16;
                    strncpy(sub_buff, buff + k, 14);
                    add_stop_char(sub_buff, 14);
                    obs->obs[obs->sv_num].L[j] = atof(sub_buff);
                    if (fabs(obs->obs[obs->sv_num].L[j]) > 0.001)
                    {
                        obs->obs[obs->sv_num].L_status[j] = USE;
                    }
                    else
                    {
                        obs->obs[obs->sv_num].L_status[j] = NOT_USE;
                    }

                    /* LLI */
                    strncpy(sub_buff, buff + k +14, 1);
                    add_stop_char(sub_buff, 1);
                    obs->obs[obs->sv_num].LLI[j] = atoi(sub_buff);

                    /* signal strength */
                    k += 16;
                    strncpy(sub_buff, buff + k, 14);
                    add_stop_char(sub_buff, 14);
                    obs->obs[obs->sv_num].D[j] = atof(sub_buff);
                    if (fabs(obs->obs[obs->sv_num].D[j]) > 0.001)
                    {
                        obs->obs[obs->sv_num].D_status[j] = USE;
                    }
                    else
                    {
                        obs->obs[obs->sv_num].D_status[j] = NOT_USE;
                    }

                    /* signal strength */
                    k += 16;
                    strncpy(sub_buff, buff + k, 14);
                    add_stop_char(sub_buff, 14);
                    obs->obs[obs->sv_num].CN0[j] = atof(sub_buff);

                    /* set wave length */
                    obs->obs[obs->sv_num].lam[j] = lam[j];
                }
            }
            obs->sv_num++;
        }
        return RET_SUCCESS;
    }

    *is_run = false;
    return RET_FAIL;
}

RETURN_STATUS load_curr_rinex_obs(opt_file_t *opt_file, obs_epoch_t *obs, uint8_t *is_open_obs_file, uint8_t *is_run)
{
    if (!read_rinex_obs_header(opt_file->obs_file, &obs->rcv_info, is_open_obs_file))
    {
        // TODO: do something
        return RET_FAIL;
    }
    
    if (!read_rinex_obs_body(opt_file, obs, is_run))
    {
        // TODO: do something
        return RET_FAIL;
    }
    
    return RET_SUCCESS;
}


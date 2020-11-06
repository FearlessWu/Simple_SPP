/**
  **********************************(C) COPYRIGHT 2020 Wyatt Wu/PQiu***********************************
  * @file        io.c/h
  * @brief       The header file is used to define various input and output functions, and the specific 
  *              function implementation is written in the io.c file
  * @note
  * @history
  * Version      Date               Author               Modification
  * V1.0.0       Oct-25-2020     Wyatt Wu/PQiu        1. establih this file
  @verbatim
  =====================================================================================================

  =====================================================================================================
  @endverbatim
  ************************************(C) COPYRIGHT 2020 Wyatt Wu/PQiu***********************************
  */

#pragma once
#include <string.h>
#include "common.h"
#include "spp_sol.h"

extern RETURN_STATUS read_opt_file(opt_file_t *opt_file, char *opt_path);

extern RETURN_STATUS read_default_opt_file(opt_file_t *opt_file);

extern void open_log_file();

extern void close_log_file();

extern RETURN_STATUS read_option_file(opt_file_t *opt_file, int32_t args, char *opt_file_path);

extern RETURN_STATUS read_rinex_nav_data(char *nav_file_path, sys_ion_cor_t *all_ion_cor, eph_t *all_eph_info, uint8_t *is_open_nav_file);

extern RETURN_STATUS load_curr_rinex_obs(char *obs_file_path, obs_epoch_t *obs, uint8_t *is_open_obs_file, uint8_t *is_run);

extern void print_log(fp64* time, error_code_t err_code, const char* message);
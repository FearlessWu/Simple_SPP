#pragma once
#include "common.h"
#include "spp_sol.h"

extern RETURN_STATUS read_opt_file(opt_file_t *opt_file, char *opt_path);

extern RETURN_STATUS read_default_opt_file(opt_file_t *opt_file);

extern void open_log_file();

extern void close_log_file();

extern RETURN_STATUS read_option_file(opt_file_t *opt_file, int32_t args, char *opt_file_path);

extern RETURN_STATUS load_curr_rinex_obs(char *obs_file_path, obs_epoch_t *obs, uint8_t *is_open_obs_file, uint8_t *is_run);

extern RETURN_STATUS load_broadcast_eph(char *nav_file_path, eph_t *eph);

/**
  * @brief      record error type and its cause to log file
  * @author     Wyatt wu
  * @param[in]  err_level:  error serious level: WARNING --program can still run,  FATAL --program can not run
  * @param[in]  err_code :  error code. According to error code, we can report the error cause to user.
  * @retval 
 */
extern void record_to_log(error_level_t err_level, error_code_t err_code);
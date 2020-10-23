#pragma once
#include "common.h"
#include "spp_sol.h"

extern RETURN_STATUS read_opt_file(opt_file_t *opt_file, char* opt_path);

extern RETURN_STATUS read_default_opt_file(opt_file_t *opt_file);

extern void open_log_file(log_t *log);

extern void close_log_file(log_t* log);

/**
  * @brief      record error type and its cause to log file
  * @author     Wyatt wu
  * @param[in]  err_level:  error serious level: WARNING --program can still run,  FATAL --program can not run
  * @param[in]  err_code :  error code. According to error code, we can report the error cause to user.
  * @retval 
 */
extern void record_to_log(log_t *log, error_level_t err_level, error_code_t err_code);
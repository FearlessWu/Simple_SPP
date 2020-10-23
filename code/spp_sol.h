/**
  ***************************************(C) COPYRIGHT 2020 Wyatt Wu***********************************
  * @file        spp_sol.c/h
  * @brief       This head file is used to define global struct, global variable, external function.
  *              Mostly about SPP solution function declear here. We also mantain a enumeration type
  *              of error code to record error type for debugging.
  * @note        
  * @history
  * Version      Date            Author          Modification
  * V1.0.0       Oct-24-2020     Wyatt Wu        1. build this file and some fundational content.
  * 
  @verbatim
  =====================================================================================================

  =====================================================================================================
  @endverbatim
  ***************************************(C) COPYRIGHT 2020 Wyatt Wu***********************************               
*/
#pragma once
#include "stdio.h"
#include "common.h"
#include "stdbool.h"
#include "stdlib.h"

#define FREQ_NUM    (2)


typedef struct
{
    uint32_t    sys_id;
    uint32_t    sv_id;
    fp64        tran_time;
    fp64        P[FREQ_NUM];
    fp64        L[FREQ_NUM];
    fp64        D[FREQ_NUM];
    int8_t      P_status[FREQ_NUM];
    int8_t      L_status[FREQ_NUM];
    int8_t      D_status[FREQ_NUM];
    fp64        CN0[FREQ_NUM];
    fp64        elv[FREQ_NUM];
    uint8_t     LLI[FREQ_NUM]; 
} obs_t;

typedef struct
{
    fp64  pos[3];
    fp64  vel[3];
    fp64  dt[2];
    fp64  pos_neu[3];
    fp64  vel_neu[3];
    fp64  blh[3];
    fp64  rcv_time;
} spp_sol_t;

typedef struct
{
    char* obs_file[256];
    char* nav_file[256];
} opt_file_t;

typedef enum
{
    NOT_USE = 0,
    USE     = 1
} obs_status_t;

typedef enum
{
    RET_FAIL    = 0,
    RET_SUCCESS = 1
} ret_status_t;

typedef enum
{
    NO_OBS_FILE = 0,
    NO_NAV_FILE = 1
} error_code_t;

typedef enum
{
    WARNING = 0,
    FATAL    = 1
} error_level_t;




typedef struct
{
    bool_t is_open;
    FILE *log_file;
} log_t;

/* global variable */
extern log_t log;
extern opt_file_t opt_file;
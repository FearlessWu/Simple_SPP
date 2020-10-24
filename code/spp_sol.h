/**
  ***************************************(C) COPYRIGHT 2020 Wyatt Wu***********************************
  * @file        spp_sol.c/h
  * @brief       This head file is used to define global struct, global variable, external function.
  *              Mostly about SPP solution function declare here. We also maintain a enumeration type
  *              of error code to record error type for debugging.
  * @note        
  * @history
  * Version      Date            Author          Modification
  * V1.0.0       Oct-24-2020     Wyatt Wu        1. build this file and some foundational content.
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

#define FREQ_NUM    (2)     // number of frequency
#define MAXOBS      (256)   // the maximum observation number
#define MAXBROEPH   (288)   // the maximun number of epoch of broadcast ephemeris

#define MAXGPSNUM   (32)
#define MAXGLONUM   (27)
#define MAXGALNUM   (30)
#define MAXBDSNUM   (50)

/* observation of a single satellite */
typedef struct
{
    fp64        time;               /*!< recevier observes time*/
    uint32_t    sys_id;
    uint32_t    sv_id;
    fp64        tran_time;          /*!< satellite signal transmit time */
    fp64        P[FREQ_NUM];
    fp64        L[FREQ_NUM];
    fp64        D[FREQ_NUM];
    int8_t      P_status[FREQ_NUM]; /*!< the status of pseudorange. It can show whether pseudorange can use or not */
    int8_t      L_status[FREQ_NUM]; /*!< the status of phase. It can show whether phase can use or not */
    int8_t      D_status[FREQ_NUM]; /*!< the status of doppler. It can show whether doppler can use or not */
    fp64        CN0[FREQ_NUM];
    fp64        elv[FREQ_NUM];
    uint8_t     LLI[FREQ_NUM]; 
} obs_sv_t;

/* all satellite observation in one epoch */
typedef struct
{
    int32_t     obs_num;        /*!< the number of actual observation */
    obs_sv_t    obs[MAXOBS];    /*!< the maximum array to restore all observation*/
} obs_epoch_t;

typedef struct
{
    fp64    time;
    // TODO: satellite orbit parameters
} eph_sat_t;

typedef struct
{
    eph_sat_t gps_eph[MAXGPSNUM][MAXBROEPH];
    eph_sat_t glo_eph[MAXGLONUM][MAXBROEPH];
    eph_sat_t gal_eph[MAXGALNUM][MAXBROEPH];
    eph_sat_t bds_eph[MAXBDSNUM][MAXBROEPH];
    // TODO: other peremters
} eph_t;

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
    char obs_file[256];
    char nav_file[256];
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
    NO_OBS_FILE        = 0,
    NO_NAV_FILE        = 1,
    CANT_READ_OPT_FILE = 2
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
/**
  ***************************************(C) COPYRIGHT 2020 Wyatt Wu***********************************
  * @file        spp_sol.c/h
  * @brief       This head/source file is used to define global struct, global variable, external function.
  *              Mostly about SPP solution function declare here. We also maintain a enumeration type of 
  *              error code to record error type for debugging.
  * @note        
  * @history
  * Version      Date            Author          Modification
  * V1.0.0       Oct-24-2020     Wyatt Wu        1. build this file and some foundational content.
  * V1.0.0       Oct-25-2020     PQiu            1. build the struct of eph.
  ***************************************(C) COPYRIGHT 2020 Wyatt Wu***********************************               
*/
#pragma once

#include "common.h"
#include "lib.h"

#define MAXSYS          (4)  // number of system
#define FREQ_NUM        (2)  // number of frequency
#define MAXOBS          (256)// the maximum observation number
#define MAXBROEPH       (30) // the maximun number of epoch of broadcast ephemeris

#define MAXGPSNUM       (32)
#define MAXGLONUM       (27)
#define MAXGALNUM       (30)
#define MAXBDSNUM       (50)

#define SYS_NON         (0x00)
#define SYS_GPS         (0x01)
#define SYS_GLO         (0x02)
#define SYS_GAL         (0x04)
#define SYS_BDS         (0x08)
#define VERY_BIG_NUM    (99999999.0)
#define VERY_SMALL_NUM  (-99999999.0)



/* observation of a single satellite */
typedef struct
{
    int32_t     sys_id;
    int32_t     sv_id;
    fp64        P[FREQ_NUM];
    fp64        L[FREQ_NUM];
    fp64        D[FREQ_NUM];
    int32_t     P_status[FREQ_NUM]; /*!< the status of pseudorange. It can show whether pseudorange can use or not */
    int32_t     L_status[FREQ_NUM]; /*!< the status of phase. It can show whether phase can use or not */
    int32_t     D_status[FREQ_NUM]; /*!< the status of doppler. It can show whether doppler can use or not */
    fp64        CN0[FREQ_NUM];      /*!< signal strength */
    fp64        azel[2];
    int32_t     LLI[FREQ_NUM]; 
} obs_sv_t;


/* receiver information */
typedef struct
{
    char sta_name[60];
    char rcv_type[20];
    fp64 appro_pos[3];          /*!< approximate ecef pos */
    fp64 atx_offset[3];         /*!< antenna delta: H/E/N */
    char gps_obs_type[20][4];
    char glo_obs_type[20][4];
    char gal_obs_type[20][4];
    char bds_obs_type[20][4];
} rcv_info_t;

/* all satellite observation in one epoch */
typedef struct
{
    fp64        time;           /*!< recevier observes time */
    fp64        ep[6];          /*!< epoch time */
    uint8_t     epoch_flag;     /*!< 0: OK, 1:power failure between previous and current epoch */
    fp64        rcv_clk_offset; /*!< Receiver clock offset, uint: sec */
    int32_t     obs_num;        /*!< the number of actual observation */
    obs_sv_t    obs[60];        /*!< all satellite obs data in obs file */
    rcv_info_t  rcv_info;       /*!< record receiver information */
} obs_epoch_t;


typedef struct
{
    uint8_t     nav_valid;     /*!< 1: nav file is intact; 0: nav file has some wrong*/
    int32_t     time[5];       /*!< Year, Month, Day, Hour, Minute*/
    uint32_t    sys_id;
    uint32_t    sv_id;          
    fp64        Toe;           /*!< ephemeris reference time*/
    fp64        M0;            /*!< 16 parameters of eph including Toe*/
    fp64        DeltaN;
    fp64        E;
    fp64        rootA;
    fp64        Omega0;
    fp64        I0;
    fp64        Omega;
    fp64        OmegaDot;
    fp64        Idot;
    fp64        CuC;
    fp64        CuS;
    fp64        CrC;
    fp64        CrS;
    fp64        CiC;
    fp64        CiS;
    fp64        Toc;           /*!< clock data reference time in seconds*/
    fp64        sv_clk[3];     /*!< the SV clock bais, drift and drift rate: Af0, Af1, Af2 */
    fp64        Tgd;
    fp64        sv_acc;
    fp64        sv_hea;

    // TODO: satellite orbit parameters of bds, glo and gal 
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
    uint32_t    sys_id;
    uint32_t    sv_id;
    fp64        satpos[3];
    fp64        satvel[3];
    fp64        satclk[2];
    fp64        el;
    fp64        transtime;
    fp64        var;
} single_sat_info_t;



typedef struct
{
    fp64        alph[4];
    fp64        beta[4];
} sin_sys_ion_cor_t;

typedef struct
{
    sin_sys_ion_cor_t     gps_ino_cor;
    sin_sys_ion_cor_t     glo_ino_cor;
    sin_sys_ion_cor_t     gal_ino_cor;
    sin_sys_ion_cor_t     bds_ino_cor;
}sys_ion_cor_t;

typedef struct
{
    uint64_t            satmask[4];
    sys_ion_cor_t       sys_ion_cor;
    single_sat_info_t   gps_sat[MAXGPSNUM];
    single_sat_info_t   glo_sat[MAXGLONUM];
    single_sat_info_t   gal_sat[MAXGALNUM];
    single_sat_info_t   bds_sat[MAXBDSNUM];
    // TODO: other peremters
} sat_info_t;

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

/* error message accord to error code */
static const char *error_message[] = {
    "FATAL: NO_OBS_FILE",
    "FATAL: NO_NAV_FILE",
    "FATAL: CANT_READ_OPT_FILE"
};

typedef enum
{
    WARNING = 0,
    FATAL   = 1
} error_level_t;


typedef struct
{
    bool_t  is_open;
    FILE   *log_fp;
} log_t;

/* global variable */
extern log_t      loger;
extern opt_file_t opt_file;
extern FILE      *obs_fp_ptr;

/*
 *@brief        the main process root function
 *@author       wyatt.wu
 *@param[in]    option file path
 *@retval       RET_FALI/RET_SUCCESS
 **/
extern RETURN_STATUS proc(opt_file_t* opt_file);

extern RETURN_STATUS LSQ(fp64 *H, fp64 *R, fp64 *v, fp64 *dx, fp64 *P);

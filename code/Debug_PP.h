/**
  *************************************(C) COPYRIGHT 2021 PQiu**************************************
  * @file        Debug_PP.h
  * @brief       This Header File declare the Debug function and cfg for PP.
  * @note
  * @history
  * Version      Date               Author               Modification
  * V0.0.1       Apr-11-2021        MXT/PQiu             1. establih/normalize this file
  * V0.0.2       Apr-11-2021        MXT/PQiu             2. declare some Debug cfg and function
  *************************************(C) COPYRIGHT 2021 PQiu**************************************
  */

#ifndef _Debug_PP_H_    /* prevent duplicate inclusion */
#define _Debug_PP_H_

#define PP_DBG    /* Control all debug info */

#ifdef PP_DBG
#define LOAD_NAV_DATA_DBG    /* debug when load BB MeaNav data */
#endif

/* value and function declaration */
extern FILE *fp_dbg_all;
#ifdef LOAD_NAV_DATA_DBG
extern FILE *fp_dbg_LoadNav;
#endif
extern void Debug_Output_PP(FILE* fp_dbgout, char* fmt, ...);
extern void Debug_Output_init();

#endif

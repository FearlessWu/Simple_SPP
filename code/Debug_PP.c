/**
  *************************************(C) COPYRIGHT 2021 PQiu**************************************
  * @file        Debug_PP.c
  * @brief       This Source File do the Debug function for PP.
  * @note
  * @history
  * Version      Date               Author               Modification
  * V0.0.1       Apr-11-2021        MXT/PQiu             1. establih/normalize this file
  * V0.0.2       Apr-11-2021        MXT/PQiu             2. realize Debug func
  *************************************(C) COPYRIGHT 2021 PQiu**************************************
  */

#pragma warning(disable : 4996)
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "Debug_PP.h"

#ifdef PP_DBG

#define DBGFILE "D:\\TestData\\debug.log"
FILE *fp_dbg_all = NULL;

#ifdef LOAD_NAV_DATA_DBG
#define DBGFILE_LOAD_NAV ("D:\\TestData\\debug_loadnav.log")
FILE *fp_dbg_LoadNav = NULL;
#endif

#endif

void Debug_Output_init()
{
#ifdef PP_DBG
    char Output_File[256] = DBGFILE;
    fp_dbg_all = fopen(Output_File, "w");

#ifdef LOAD_NAV_DATA_DBG
    char Output_File_LoadNav[256] = DBGFILE_LOAD_NAV;
    fp_dbg_LoadNav = fopen(Output_File_LoadNav, "w");
#endif

#endif
}

void Debug_Output_PP(FILE* fp_dbg, char* fmt, ...)
{
    char    out[2048] = {0};     /* output for debug */
    va_list body      = NULL;    /* out */

    va_start(body, fmt);
    vsprintf(out, fmt, body);
    va_end(body);

    if (fp_dbg != NULL)
    {
        fwrite(out, strlen(out), 1, fp_dbg);
        printf(out); // print to screen
    }
    else
    {
        printf("ERROR: Debug File is NULL!");
    }
}


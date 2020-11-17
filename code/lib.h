/**
  ***************************************(C) COPYRIGHT 2020 Wyatt Wu***********************************
  * @file        lib.c/h
  * @brief       This head/source file is used to write various basic library functions(mainly  
  *              from RTKLIB), basic macro definitions, and clear definitions of basic types
  * @note
  * @history
  * Version      Date            Author          Modification
  * V1.0.0       Nov-07-2020     Wyatt Wu        1. build this file and some foundational content.
  ***************************************(C) COPYRIGHT 2020 Wyatt Wu***********************************
*/
#pragma once
#include <math.h>
#include "Matrix.h"
#include "common.h"

#define OMGE        (7.2921151467E-5)       /* earth angular velocity (IS-GPS) (rad/s)*/
#define FE_WGS84    (1.0/298.257223563)     /* earth flattening (WGS84) */
#define RE_WGS84    (6378137.0)             /* earth semimajor axis (WGS84) (m)*/
#define PI          (3.1415926535897932)    /* pi */
#define CLIGHT      (299792458.0)           /* light speed */


/**
  * @brief      convert calendar day/time to time
  * @author     quote from RTKLIB, modified by Wyatt wu
  * @param[in]  ep: day/time {year,month,day,hour,min,sec}
  * @retval     fp64 time
 */
extern fp64 epoch2time(const fp64 *ep);

/**
  * @brief      time to calendar day/time 
  * @author     quote from RTKLIB, modified by Wyatt wu
  * @param[in]  time: 
  * @param[out] ep  : day/time {year,month,day,hour,min,sec}
 */
extern void time2epoch(fp64 time, fp64 *ep);


/**
  * @brief         convert time to week and tow in gps time
  * @author        quote from RTKLIB, modified by Wyatt wu
  * @param[in]     t    : gps time in 1970
  * @param[in/out] week : week number in gps time (NULL: no output)
  * @retval        time of week in gps time (s)
 */
extern fp64 time2gpst(fp64 t, int32_t *week);

/* 
 * @brief        inner product of vectors
 * @auther       quote from RTKLIB
 * @param[in]    a:vector a (n x 1)
 * @param[in]    b:vector b (n x 1)
 * @param[in]    n: size of vector a,b
 * @retval       a'*b
 *-----------------------------------------------------------------------------
*/
extern fp64 dot(const fp64 *a, const fp64 *b, int32_t n);

/*
 *@brief        euclid norm of vector
 *@auther       quote from RTKLIB
 *@param[in]    a : vector a (n x 1)
 *@param[in]    n : size of vector a
 *@retval       || a ||
 **/
extern fp64 norm(const fp64* a, int32_t n);

/*
 *@brief        transform ecef to geodetic postion
 *@auther       quote from RTKLIB
 *@param[in]    xyz: ecef position {x,y,z} (m)
 *@param[out]   blh: geodetic position {lat,lon,h} (rad,m)
 *@retval       none
 **/
extern void xyz2blh(const fp64 *xyz, fp64 *blh);

/*
 *@brief        eometric distance and receiver-to-satellite unit vector
 *@auther       quote from RTKLIB
 *@param[in]    rs: satellilte position (ecef at transmission) (m)
 *@param[in]    rr: receiver position (ecef at reception) (m)
 *@param[out]   e : line-of-sight vector (ecef)
 *@retval       geometric distance (m) (0>:error/no satellite position)
 *@note         distance includes sagnac effect correction
 **/
extern fp64 geodist(const fp64* rs, const fp64* rr, fp64* e);

/*
 *@brief        compute ecef to local coordinate transfromation matrix
 *@auther       quote from RTKLIB
 *@param[in]    blh: geodetic position {lat,lon} (rad)
 *@param[out]   E  : ecef to local coord transformation matrix (3x3)
 *@retval       none
 **/
extern void xyz2enu(const fp64 *blh, fp64 *E);

/*
 *@brief        transform ecef vector to local tangental coordinate
 *@author       quote from RTKLIB
 *@param[in]    blh: geodetic position {lat,lon} (rad)
 *@param[in]    r  : vector in ecef coordinate {x,y,z}
 *@param[out]   e  : vector in local tangental coordinate {e,n,u}
 *@retval       none
 * */
extern void ecef2enu(const fp64 *blh, const fp64 *r, fp64 *e);

/*
 *@brief        compute satellite azimuth/elevation angle
 *@auther       quote from RTKLIB
 *@param[in]    blh : geodetic position {lat,lon,h} (rad,m)
 *@param[in]    e   : receiver-to-satellilte unit vevtor (ecef)
 *@param[out]   azel: azimuth/elevation {az,el} (rad) (NULL: no output)
 *@retval       elevation angle (rad)
 **/
extern fp64 satazel(const fp64 *blh, const fp64 *e, fp64 *azel);
/**
  ***************************************(C) COPYRIGHT 2020 Wyatt Wu***********************************
  * @file        common.c/h
  * @brief       This head/source file is used to define various public functions and macro definitions 
  *              for SPP and PPP et al
  * @note
  * @history
  * Version      Date            Author          Modification
  * V1.0.0       Nov-07-2020     Wyatt Wu        1. build this file and some foundational content.
  ***************************************(C) COPYRIGHT 2020 Wyatt Wu***********************************
*/
#pragma once
#include "lib.h"

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
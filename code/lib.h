#include "common.h"

/**
  * @brief      convert calendar day/time to time
  * @author     quote from RTKLIB, modified by Wyatt wu
  * @param[in]  ep: day/time {year,month,day,hour,min,sec}
  * @retval     fp64 time
 */
extern fp64 epoch2time(const double *ep);
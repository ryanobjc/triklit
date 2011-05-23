#include "CONF/config.h"
#include "CONF/common.h"

/* takes number of milliseconds to run for, or 0 for default time,
   and options string (or NULL) */
effect(
  int msecs,
  char *options
)
{
  int timec, timet;
  int ix, iy, iz;

  if (msecs == 0) msecs = 15 * 1000;

  v_all_off(v);
  vw_all_set(v,GREEN);
  TL_frame(v,1);
  while (v_dim(v,0.98))
    TL_frame(v,100);
}

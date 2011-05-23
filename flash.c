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
  for (timec = 0; timec < msecs;)
  {
    vw_all_set(v,RED);
    timec += TL_frame(v,1000);
    vw_all_set(v,GREEN);
    timec += TL_frame(v,1000);
    vw_all_set(v,BLUE);
    timec += TL_frame(v,1000);
    vw_all_set(v,WHITE);
    timec += TL_frame(v,1000);
    v_all_off(v);
    timec += TL_frame(v,1000);
  }
}

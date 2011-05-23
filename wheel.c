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
  int wi;
 
  if (msecs == 0) msecs = 20000;

  v_all_off(v);

  wi = 0;
  for (timec = 0; timec < msecs;)
  {
    vw_all_set(v,wheel[wi % wheel_max].u.w);
    wi++;
    timec += TL_frame(v,10);
  }
  v_all_off(v);
  TL_frame(v,1);
}

#include "CONF/config.h"
#include "CONF/common.h"

/* takes number of milliseconds to run for, or 0 for default time,
   and options string (or NULL) */
effect(
  int msecs,
  char *options
)
{
  v_all_off(v);
  TL_frame(v,100);
}

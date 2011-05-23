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
  int wi, string,light;
 
  if (msecs == 0) msecs = 15000;

  v_all_off(v);
  wi = 0;

  for (timec = 0; timec < msecs;)
  {
    for (light = 0; light < SY; light++)
      {
	wi += 16;
	for (string = 0; string < SX; string++)
	  vw(v,string,light,0) = wheel[wi % wheel_max].u.w;
	timec += TL_frame(v,100);
      }
    v_all_off(v);
    TL_frame(v,1);
  }
}


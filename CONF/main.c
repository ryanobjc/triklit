/*
  main function for all effects.
  usage: effect [time [options]]

    time is number of seconds or millseconds to run effect, 0=default
    options is an option specific string to pass to effect.
*/


#include <stdio.h>
#include "config.h"
#include "common.h"

main
(
 int argc,
 char *argv[]
)
{
  int msecs;
  char *options;

  if (!TL_init())
  {
    fprintf(stderr,"Can't init output module\n");
    exit(1);
  }
  if (!TL_open())
  {
    fprintf(stderr,"Can't open output\n");
    exit(1);
  }
  common_init();

  msecs = 0;
  options = "0";
  if (argc > 1)
    msecs = atoi(argv[1]);
  if (argc > 2)
    options = argv[2];
  if (msecs < 1000)
    msecs = msecs * 1000;  // convert to milliseconds

  effect(msecs,options);

  TL_close();
}

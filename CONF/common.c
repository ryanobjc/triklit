/*
 common routines

 common_init()            init anything needed by all functions in this module

 v_all_off(v)             set all voxels off
 v_all_set(v,r,g,b)       set all voxels to r,g,b value
 vw_all_set(v,w)          set all voxels to w value
 v_dim(v,percent)         dim all voxels by this much (0.0-1.0)
*/

#include <stdio.h>
#include "config.h"
#include "common.h"
#ifdef BSD
#include "math.h"
#endif

#define SLEN 256

common_init()
{
  int ix, iy, iz;

  srandom((unsigned long) time(NULL) & 0x001f);

  /* init blank voxel array */
  for (ix = 0; ix < SX; ix++)
    for (iy = 0; iy < SY; iy++)
      for (iz = 0; iz < SZ; iz++)
	vw(VZ,ix,iy,iz) = 0;

  /* set maplen to maximum dimension */
  if (SX > maplen) maplen = SX;
  if ((SY*2+1) > maplen) maplen = SY*2+1;
  if (SZ > maplen) maplen = SZ;

  smaplen = sqrt( pow(X,2) + pow(Y,2) + pow(Z,2) ) + 1;
  if (maplen > smaplen) smaplen = maplen;

  init_wheels();
}

init_wheels()
{
  init_wheelstd();
  init_wheelsat();
  init_wheelavg();
  set_wheelavg();  // good for LEDS
}

init_wheelavg()
{
  int i, s;

  for (i = 0; i < wheelstd_max; i++)
  {
    s = (wheelstd[i].u.b.r + wheelsat[i].u.b.r) / 2;
    wheelavg[i].u.b.r = s;
    s = (wheelstd[i].u.b.g + wheelsat[i].u.b.g) / 2;
    wheelavg[i].u.b.g = s;
    s = (wheelstd[i].u.b.b + wheelsat[i].u.b.b) / 2;
    wheelavg[i].u.b.b = s;
  }
  wheelavg_max = wheelstd_max;
}

set_wheelavg()
{
  wheel = wheelavg;
  wheel_max = wheelavg_max;
}

int wheel_std(n)
  int n;
{
  if (n < 256) return(n);
  if (n < 512) return((511-n));
  return(0);
}

init_wheelstd()
{
  int i,c,s;
  float d;

  for (c = 0; c < 768; c++)
  {
    wheelstd[c].u.b.r=wheel_std((c+256) % 768);
    wheelstd[c].u.b.g=wheel_std(c);
    wheelstd[c].u.b.b=wheel_std((c+512) % 768);
  }
  wheelstd_max = c;
}

/* a more saturated color wheel */
init_wheelsat()
{
  double n, s, p;
  int i, l, si;

  l = 256 * 2;
  p = (2.0 * 3.14159) / (256.0 * 2.0);
  n = 4.712385;
  for (i = 0; i < l; i++)
  {
    s = 128.0 * (sin(n) + 1.0);
    si = s;
    n = n + p;
    wheelsat[i].u.b.g = si;
  }
  for (; i < (256 * 3); i++)
  {
    wheelsat[i].u.b.g = 0;
  }
  wheelsat_max = i;
  for (i = 0; i < wheelsat_max; i++)
  {
    wheelsat[i].u.b.r = wheelsat[(i+256) % wheelsat_max].u.b.g;
    wheelsat[i].u.b.b = wheelsat[(i+512) % wheelsat_max].u.b.g;
  }
}

/* set the entire array to a single color */
v_all_set(v,r,g,b)
  struct color v[SX][SY][SZ];
  byte r, g, b;
{
  int x, y, z;

  for (x = 0; x < SX; x++)
    for (y = 0; y < SY; y++)
      for (z = 0; z < SZ; z++)
      {
	vr(v,x,y,z) = r;
	vg(v,x,y,z) = g;
	vb(v,x,y,z) = b;
	vx(v,x,y,z) = 0;
      }
}

vw_all_set(v,w)
  struct color v[SX][SY][SZ];
  word w;
{
  int x, y, z;

  for (x = 0; x < SX; x++)
    for (y = 0; y < SY; y++)
      for (z = 0; z < SZ; z++)
	vw(v,x,y,z) = w;
}

/* turn everything off */
v_all_off(v)
  struct color v[SX][SY][SZ];
{
  vw_all_set(v,0);
}

/* dim pixels in array by p%, return count of pixels still on */
int v_dim(v,p)
  struct color v[SX][SY][SZ];
  float p;
{
  int x, y, z, on;
  int r, g, b;

  on = 0;
  for (x = 0; x < SX; x++)
    for (y = 0; y < SY; y++)
      for (z = 0; z < SZ; z++)
      {
	r = vr(v,x,y,z) * p;
	g = vg(v,x,y,z) * p;
	b = vb(v,x,y,z) * p;
	vr(v,x,y,z) = r;
	vg(v,x,y,z) = g;
	vb(v,x,y,z) = b;
	if (r+g+b) on++;
      }
  return(on);
}

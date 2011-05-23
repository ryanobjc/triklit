/*
  Triklits output module, v1.0.  2006-09-01

  (C) Copyright 2006 by Network Wizards

  generate rgb data frames for Triklits,
  output them to a file, USB or network controller.

 routines:
 int TL_init()
    call once to initialize the module. return TRUE if ok.
 int TL_open()
    call to open output device.  return TRUE if ok.
 TL_close()
    call to close output device
 TL_frame(v,msecs)
    generate and output data frames to device
 TL_mode(bits)
    call to set output modes (not needed)
*/

#include "config.h"
#ifdef BSD
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#endif
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <termios.h>
#include "common.h"

//----------------------------------------------------------------------
//  the following items need to be configured for the output device


// pick the output device:
// uncomment one of the following lines
#define OUT_FILE        // to file or com port (or USB via COM)
//#define OUT_NET         // to tcp/ip connection (for Ethernet driver)

// if OUT_FILE selected:
// the output file name is formed from the printf format string
// OUT_NAME_FORMAT.   it can contain a format specifier for the
// output device number (which is increment by 1 for each device).
// sample strings:
// for a COM port (COMn)
//   #define OUT_NAME_FORMAT "/dev/com4"          (if one output device only)
//   #define OUT_NAME_FORMAT "/dev/com%d"         (COMn where N starts at OUT_NAME_NUM)
//   #define OUT_NAME_FORMAT "/dev/cu.usbmodem3B11"  (MAC format device name)
//   #define OUT_NAME_FORMAT "/dev/ttyp%x"
// for a file name
//   #define OUT_NAME_FORMAT "out.bin"       (if one output device only)
//   #define OUT_NAME_FORMAT "out%d.bin"

#define OUT_NAME_FORMAT "/dev/cu.usbmodem3d11"        // define this to be COM port of USB4 interface
#define OUT_NAME_NUM  1               // base output number to start at


// if OUT_NET is selected:
// the IP address if formed from OUT_HOST_NET and OUT_HOST NUM.
// OUT_HOST_NUM is iincremented by 1 for each output device
#define OUT_HOST_NET   "192.168.0."
#define OUT_HOST_NUM   200
#define OUT_PORT 1000

// ignore this
//#define LPR_PROTOCOL     
//#define LPR_NAME   "lp"

#define FTIME                   19    // milliseconds to output one frame


//maximum number of light strings
#define OUTPUT_STRINGS            4    // must be <= (OUTPUT_DEVICES * OUTPUT_INTERLEAVE * 8)
#define OUTPUT_DEVICES            1    // number of controllers
#define OUTPUT_INTERLEAVE         1    // number of driver boards per controller
#define SLOTS                     24   // number of lights on a string
//uncomment the line below for 4-port USB interface
#define OUTPUT_4BITS                 // pack 4-bit nibbles into bytes
#define USBCDC                       // set this if using USB interface

//   end of output device configuration section
//----------------------------------------------------------------------





#define FRAMELEN  ((OUTPUT_INTERLEAVE * ((SLOTS*32) + 8)) + 2)  
// each light needs 32 bits to output the rgb values
// 8 is length of header+trailer bits (4+4)
// 2 is fudge space


int skt[OUTPUT_DEVICES];
FILE *fin[OUTPUT_DEVICES], *fout[OUTPUT_DEVICES];
FILE *ofp[OUTPUT_DEVICES];      /* output file descriptors */

int BITS;			/* number of bits of RGB color to use */

byte frame[OUTPUT_DEVICES][FRAMELEN];
int fc[OUTPUT_DEVICES];               // frame count/length (bytes)


#define SLEN 256   // generic string len

int TL_init()
{
  TL_mode(8);
  return(1);
}


/* set color to b bits (always 8) */
TL_mode(b)
  int b;
{
  BITS = b;
}

/*
 this function takes a string and slot location and returns the rgb value
 that should be placed there.   it is called by the frame_gen routine to
 get the rgb value for every light on every string.   it allows you to do
 things such as map a 3d cube to which string/slot each light is on.

 for example ssmap(v,4,9) asks for the rgb value to put on string 4, light 9.
 the default function gets the value from location array v[4][9][0].
 it assumes v is a 3d array where x=string number, y=light number, and z is always 0.
 you can modify this function to do more complicated mappings.
*/
word ssmap(v,string,slot)
  struct color v[SX][SY][SZ];
  int string;
  int slot;
{
  return(vw(v,string,slot,0));
}

/* generate frame(s) using current rgb array values */
frame_gen(v)
  struct color v[SX][SY][SZ];
{
  int b, d, i, slot, string, bit, ebit, s, xi, xn;
  word w;
  word xa[1+(OUTPUT_STRINGS/32)][32];

  /* output the frame header bits (0010) */
  for (d = 0; d < OUTPUT_DEVICES; d++)
  {
    fc[d] = 0;
    for (b = 0; b < OUTPUT_INTERLEAVE; b++)
      frame[d][fc[d]++] = 0;
    for (b = 0; b < OUTPUT_INTERLEAVE; b++)
      frame[d][fc[d]++] = 0;
    for (b = 0; b < OUTPUT_INTERLEAVE; b++)
      frame[d][fc[d]++] = 0xff;
    for (b = 0; b < OUTPUT_INTERLEAVE; b++)
      frame[d][fc[d]++] = 0;
  }

  for (slot = 0; slot < SLOTS; slot++)  /* for each light on the string */
  {
    xi = 0; xn = 0;
    for (string = 0; string < OUTPUT_STRINGS; string++)  /* for each string */
    {
      w = ssmap(v,string,slot);  /* get rgb value for string/slot location via map */

      // printf("map %d %d => %08x\n",string,slot,w);

      xa[xn][xi] = w;          /* add to next transposition array slot */
      xi++;
      if (xi > 31)
      {
	xi = 0;
	xn++;
      }
    }
    if (xi > 0) xn++;

    /* now transpose all the output arrays we have */
    for (i = 0; i < xn; i++)
      frame_transpose(xa[i]);

    /* now ouput the bits for this light */
      for (bit = 7; bit >= 0; bit--)
      {
	xi = 0; xn = 0;
	for (d = 0; d < OUTPUT_DEVICES; d++)
	for (b = 0; b < OUTPUT_INTERLEAVE; b++)
	{
	  /* R */
	  if (xi == 0)
	    frame[d][fc[d]++] = ((xa[xn][bit] & 0xff000000) >> 24);
	  if (xi == 1)
	    frame[d][fc[d]++] = ((xa[xn][bit] & 0x00ff0000) >> 16);
	  if (xi == 2)
	    frame[d][fc[d]++] = ((xa[xn][bit] & 0x0000ff00) >> 8);
	  if (xi == 3)
	    frame[d][fc[d]++] = ((xa[xn][bit] & 0x000000ff));
	  xi++;
	  if (xi > 3) { xi = 0; xn++; }
	}
	xi = 0; xn = 0;
	for (d = 0; d < OUTPUT_DEVICES; d++)
	for (b = 0; b < OUTPUT_INTERLEAVE; b++)
	{
	  /* G */
 	  if (xi == 0)
	    frame[d][fc[d]++] = ((xa[xn][bit+8] & 0xff000000) >> 24);
	  if (xi == 1)
	    frame[d][fc[d]++] = ((xa[xn][bit+8] & 0x00ff0000) >> 16);
	  if (xi == 2)
	    frame[d][fc[d]++] = ((xa[xn][bit+8] & 0x0000ff00) >> 8);
	  if (xi == 3)
	    frame[d][fc[d]++] = ((xa[xn][bit+8] & 0x000000ff));
	  xi++;
	  if (xi > 3) { xi = 0; xn++; }
	}
	xi = 0; xn = 0;
	for (d = 0; d < OUTPUT_DEVICES; d++)
	for (b = 0; b < OUTPUT_INTERLEAVE; b++)
	{
	  /* B */
	  if (xi == 0)
	    frame[d][fc[d]++] = ((xa[xn][bit+16] & 0xff000000) >> 24);
	  if (xi == 1)
	    frame[d][fc[d]++] = ((xa[xn][bit+16] & 0x00ff0000) >> 16);
	  if (xi == 2)
	    frame[d][fc[d]++] = ((xa[xn][bit+16] & 0x0000ff00) >> 8);
	  if (xi == 3)
	    frame[d][fc[d]++] = ((xa[xn][bit+16] & 0x000000ff));
	  xi++;
	  if (xi > 3) { xi = 0; xn++; }
	}
	xi = 0; xn = 0;
	for (d = 0; d < OUTPUT_DEVICES; d++)
	for (b = 0; b < OUTPUT_INTERLEAVE; b++)
	{
	  /* P */
	  if (xi == 0)
	    frame[d][fc[d]++] = ~((xa[xn][bit+16] & 0xff000000) >> 24);
	  if (xi == 1)
	    frame[d][fc[d]++] = ~((xa[xn][bit+16] & 0x00ff0000) >> 16);
	  if (xi == 2)
	    frame[d][fc[d]++] = ~((xa[xn][bit+16] & 0x0000ff00) >> 8);
	  if (xi == 3)
	    frame[d][fc[d]++] = ~((xa[xn][bit+16] & 0x000000ff));
	  xi++;
	  if (xi > 3) { xi = 0; xn++; }
	}
      }
  }

  /* output the frame trailer bits (0000) */
  for (d = 0; d < OUTPUT_DEVICES; d++)
  {
    for (b = 0; b < OUTPUT_INTERLEAVE; b++)
      frame[d][fc[d]++] = 0;
    for (b = 0; b < OUTPUT_INTERLEAVE; b++)
      frame[d][fc[d]++] = 0;
    for (b = 0; b < OUTPUT_INTERLEAVE; b++)
      frame[d][fc[d]++] = 0;
    for (b = 0; b < OUTPUT_INTERLEAVE; b++)
      frame[d][fc[d]++] = 0;
  }

#ifdef OUTPUT_4BITS
  pack_frames();
#endif
}

/* bit transpose a 32x32 bit array */
frame_transpose(x)
  word *x;
{
  word t;
  int i;

  // printf("transpose %08x\n",x);
  for (i = 0; i < 16; i++)
  {
    t       = (x[i]    & 0x0000ffff) << 16;
    x[i]    = (x[i]    & 0xffff0000) | ((x[i+16] & 0xffff0000) >> 16);
    x[i+16] = (x[i+16] & 0x0000ffff) | t;
  }

  for (i = 0; i < 8; i++)
  {
    t       = (x[i]    & 0x00ff00ff) << 8;
    x[i]    = (x[i]    & 0xff00ff00) | ((x[i+8] & 0xff00ff00) >> 8);
    x[i+8]  = (x[i+8]  & 0x00ff00ff) | t;

    t       = (x[i+16] & 0x00ff00ff) << 8;
    x[i+16] = (x[i+16] & 0xff00ff00) | ((x[i+24] & 0xff00ff00) >> 8);
    x[i+24] = (x[i+24] & 0x00ff00ff) | t;
  }

  for (i = 0; i < 4; i++)
  {
    t       = (x[i]    & 0x0f0f0f0f) << 4;
    x[i]    = (x[i]    & 0xf0f0f0f0) | ((x[i+4] & 0xf0f0f0f0) >> 4);
    x[i+4]  = (x[i+4]  & 0x0f0f0f0f) | t;

    t       = (x[i+8]  & 0x0f0f0f0f) << 4;
    x[i+8]  = (x[i+8]  & 0xf0f0f0f0) | ((x[i+12] & 0xf0f0f0f0) >> 4);
    x[i+12] = (x[i+12] & 0x0f0f0f0f) | t;

    t       = (x[i+16] & 0x0f0f0f0f) << 4;
    x[i+16] = (x[i+16] & 0xf0f0f0f0) | ((x[i+20] & 0xf0f0f0f0) >> 4);
    x[i+20] = (x[i+20] & 0x0f0f0f0f) | t;

    t       = (x[i+24] & 0x0f0f0f0f) << 4;
    x[i+24] = (x[i+24] & 0xf0f0f0f0) | ((x[i+28] & 0xf0f0f0f0) >> 4);
    x[i+28] = (x[i+28] & 0x0f0f0f0f) | t;
  }

  for (i = 0; i < 30; i += 4)
  {
    t       = (x[i]    & 0x33333333) << 2;
    x[i]    = (x[i]    & 0xcccccccc) | ((x[i+2] & 0xcccccccc) >> 2);
    x[i+2]  = (x[i+2]  & 0x33333333) | t;

    t       = (x[i+1]  & 0x33333333) << 2;
    x[i+1]  = (x[i+1]  & 0xcccccccc) | ((x[i+3] & 0xcccccccc) >> 2);
    x[i+3]  = (x[i+3]  & 0x33333333) | t;
  }

  for (i = 0; i < 32; i += 2)
  {
    t       = (x[i]    & 0x55555555) << 1;
    x[i]    = (x[i]    & 0xaaaaaaaa) | ((x[i+1] & 0xaaaaaaaa) >> 1);
    x[i+1]  = (x[i+1]  & 0x55555555) | t;
  }
}

#ifdef OUTPUT_4BITS
// for all the output data in a frame buffer, pack 4-bit nibbles into bytes
// like so:  byte1, byte2  =>  byte1-high-4-bits | (byte2-high-4-bits >> 4)
// then adjust frame counts
// note: framecount assumed to be even.
pack_frames()
{
  int i, d;
  byte p;

  for (d = 0; d < OUTPUT_DEVICES; d++)
  {
    for (i = 0; i < fc[d]; i += 2)
    {
      p = (frame[d][i] & 0xf0) | (frame[d][i+1] >> 4);
      frame[d][i/2] = p;
    }
    fc[d] = fc[d] / 2;
  }
}
#endif

/* send the current frames */
frame_tx()
{
  int d;

  for (d = 0; d < OUTPUT_DEVICES; d++) {
    fwrite(frame[d],1,fc[d],ofp[d]);
    fflush(ofp[d]);
  }
}


/* display the voxel array v for msec milliseconds (1 for fastest possible)
   returns number of milliseconds used */
int TL_frame(v,msec)
  struct color v[SX][SY][SZ];
  int msec;
{
  int fcount, f;
  int ix, iy, iz;

  if (msec < 1) msec = 1;
  fcount = msec/FTIME;
  if (fcount < 1) fcount = 1;
  frame_gen(v);
  for (f = 0; f < fcount; f++)
    frame_tx();
  return(fcount*FTIME);
}


#ifdef OUT_FILE
int TL_open()
{
  int d;
  char fname[SLEN];
#ifdef USBCDC
  struct termios tp;
#endif

  for (d = 0; d < OUTPUT_DEVICES; d++)
  {
    sprintf(fname,OUT_NAME_FORMAT,OUT_NAME_NUM + d);
    ofp[d] = fopen(fname,"wb");
    if (ofp[d] == NULL) return(0);
#ifdef USBCDC
    // do this for USB interface (notifies USB device to reset buffers)
    tcgetattr(fileno(ofp[d]),&tp);
#ifdef MAC         
    // this might be necessary on a MAC
    tp.c_iflag |= IGNBRK;
    tp.c_cflag |= CLOCAL;
    tp.c_cflag &= ~(HUPCL | CRTSCTS);
#endif
    tcsetattr(fileno(ofp[d]),TCSANOW,&tp);
#endif
  }
  return(1);
}

TL_close()
{
  int d;

  for (d = 0; d < OUTPUT_DEVICES; d++)
    fclose(ofp[d]);
}
#endif

#ifdef OUT_NET
int TL_open()
{
  struct hostent *hp;
  int port, reply;
  struct sockaddr_in sa;
  struct sockaddr_in sin;
  u_long addr, adx;
  char fname[SLEN];
  int d;

 for (d = 0; d < OUTPUT_DEVICES; d++)
 {
  sprintf(fname,"%s%d",OUT_HOST_NET,OUT_HOST_NUM+d);
  if ((hp = gethostbyname(fname)) == NULL)
  {
    fprintf(stderr,"can't find host '%s'.\n",fname);
    return(0);
  }

  /* create a socket */
  if ((skt[d] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  {
    fprintf(stderr,"can't create socket\n");
    return(0);
  }
 
  sin.sin_family = hp->h_addrtype;
  bcopy (hp->h_addr, &sin.sin_addr, hp->h_length);
  addr = ntohl(sin.sin_addr.s_addr);
  sin.sin_port = htons(OUT_PORT);

  /* Now, connect to that port */
  if (connect(skt[d], (struct sockaddr *) &sin, sizeof(sin)) < 0)
  {
    close(skt[d]);
    fprintf(stderr,"Can't connect to server %s\n",fname);
    return(0);
  }

  /* associate a STREAM to 'fin' and 'fout' from 'skt' */
  if ((fin[d] = fdopen(skt[d], "r")) == NULL ||
      (fout[d] = fdopen(skt[d], "w")) == NULL)
  {
    close(skt[d]);
    fprintf(stderr,"Can't get stream descriptors for socket\n");
    return(0);
  }

#ifdef LPR_PROTOCOL
  fprintf(fout,"%c%s\n",0x02,LPR_NAME);
  fflush(fout);
  reply = fgetc(fin);
  if (reply != 0)
  {
    fprintf(stderr,"device not online (err %d)\n",reply);
    return(0);
  }


  fprintf(fout,"%c1000000000 dfA666nw\n",0x03);
  fflush(fout);
  reply = fgetc(fin);
  if (reply != 0)
  {
    fprintf(stderr,"Device not accepting more data (err %d)\n",reply);
    return(0);
  }
#endif

  ofp[d] = fout[d];
 }

  return(1);
}

TL_close()
{
  int d;

  for (d = 0; d < OUTPUT_DEVICES; d++)
  {
    fclose(fin[d]); fclose(fout[d]);
    close(skt[d]);
  }
}
#endif

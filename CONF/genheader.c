/*
 generate a header file for SD based controllers

 usage:  genheader outfile [address [interleave [timing]]]

  defaults are 0 1 40000



  HEADER FORMAT
    header is first 512 bytes.  it is optional (identified by signature).
    format is:
      signature:     0xcc 0x33
      address:       0x00 0xnn    0 for master clocker, else a slave
      interleave:    0x00 0xnn    number of output driver boards: 1,2,4,8 only!
      timing:        0xhh 0xmm 0xnn 0xll (32-bit output rate in Hz [hhmmnnll])
                                usually 40000 or 25000.
      rest of bytes unused for now
*/

#include <stdio.h>

main(int argc, char **argv)
{
  FILE *fp;
  int address;
  int interleave;
  long timing;
  unsigned char header[512];
  int i;

  address = 0;
  interleave = 1;
  timing = 40000;

  if (argc < 2)
  {
    printf("usage:  genheader outfile [address [interleave [timing]]]\n");
    exit(1);
  }

  if (argc > 2)
    address = atoi(argv[2]);
  if (argc > 3)
    interleave = atoi(argv[3]);
  if (argc > 4)
    timing = atoi(argv[4]);

  printf("gen header file %s, address %d, interleave %d, timing %d\n",
	 argv[1],address,interleave,timing);
  
  for (i = 0; i < 512; i++) header[i] = 0;

  header[0] = 0xcc; header[1] = 0x33;
  header[2] = 0; header[3] = address;
  header[4] = 0; header[5] = interleave;
  header[6] = (timing & 0xff000000) >> 24;
  header[7] = (timing & 0x00ff0000) >> 16;
  header[8] = (timing & 0x0000ff00) >> 8;
  header[9] = (timing & 0x000000ff);

  fp = fopen(argv[1],"w");
  fwrite(header,1,512,fp);
  fclose(fp);
}

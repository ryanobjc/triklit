/* max coords */
#define X (SX-1)
#define Y (SY-1)
#define Z (SZ-1)
#define SY2 (SY*2)

/* center coords */
#define CX (SX/2)
#define CY (SY/2)
#define CZ (SZ/2)

/* voxel addressing macros */
#define vr(va,x,y,z) va[x][y][z].u.b.r
#define vg(va,x,y,z) va[x][y][z].u.b.g
#define vb(va,x,y,z) va[x][y][z].u.b.b
#define vx(va,x,y,z) va[x][y][z].u.b.x
#define vw(va,x,y,z) va[x][y][z].u.w

typedef unsigned char byte;
typedef unsigned int word;

#define PI 3.14159

#define MC 0xff   // max color value

/* rgb data for each voxel */
/* access as a word as v[x][y][z].u.w
              bytes as v[x][y][z].u.b.(r/g/b)
*/
struct color
{
  union
  {
    word w;
    struct
    {
#ifdef LITTLEENDIAN
      byte x;
      byte b;
      byte g;
      byte r;
#else
      byte r;
      byte g;
      byte b;
      byte x;
#endif
    } b;
  } u;
};

struct color v[SX][SY][SZ];            // primary scratch array
struct color va[SX][SY][SZ];           // secondary scratch array
struct color VZ[SX][SY][SZ];           // an array that is always blank

#define WHITE      0x4098a000
//#define BLUE_WHITE 0xb0b0ff00
#define BLUE_WHITE 0x4098d000
#define RED        0xff000000
#define GREEN      0x00ff0000
#define BLUE       0x0000ff00
#define PURPLE     0xff00ff00
#define PINK       0xffa0a000
#define YELLOW     0xff700000
#define ORANGE     0xff200000
#define AQUA       0x00ff6400

#define WHEELMAX 2048
struct color *wheel;
struct color wheelstd[WHEELMAX];
struct color wheelsat[WHEELMAX];
struct color wheelavg[WHEELMAX];
int wheel_max;
int wheelstd_max;
int wheelsat_max;
int wheelavg_max;

int maplen, smaplen;

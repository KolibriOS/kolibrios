
typedef unsigned int color_t;

typedef struct
{
  int x;
  int y;
}pt_t;

/***********               Clipping             **********/

typedef struct
{
  int xmin;
  int ymin;
  int xmax;
  int ymax;
}clip_t, *PTRclip;

#define CLIP_TOP          1
#define CLIP_BOTTOM       2
#define CLIP_RIGHT        4
#define CLIP_LEFT         8

int LineClip ( clip_t *clip, int *x1, int *y1, int *x2, int *y2 );

int BlockClip( clip_t *clip, int *x1, int *y1, int *x2, int* y2 );

/***********               Brushes             ***********/

typedef struct
{
  color_t bkcolor;
  color_t fcolor;
  union {
    u32_t bmp[2];
    u8_t  bits[8];
  };
}brush_t;

#define HS_HORIZONTAL     0
#define HS_VERTICAL       1
#define HS_FDIAGONAL      2
#define HS_BDIAGONAL      3
#define HS_CROSS          4
#define HS_DIAGCROSS      5


#define HATCH_MAX         7

brush_t* CreateHatch(int hatch, color_t bkcolor, color_t fcolor);

void     DestroyBrush(brush_t *brush);

/***********         Pixmap & drawing          ***********/

typedef struct
{
  unsigned  width;
  unsigned  height;
  u32_t     format;
  u32_t     flags;
  unsigned  pitch;
  void      *mapped;
  u32_t     handle;
}pixmap_t;

#define  PX_MEM_SYSTEM    0
#define  PX_MEM_LOCAL     1
#define  PX_MEM_GART      2

#define  PX_MEM_MASK      3

#define  PX_LOCK          1

pixmap_t* CreatePixmap(unsigned width, unsigned height, u32_t format, u32_t flags);

void* LockPixmap(pixmap_t *pixmap);

int UnlockPixmap(pixmap_t *pixmap);

int DrawRect(pixmap_t *pixmap, int xorg, int yorg,
             int width, int height,
             color_t dst_color, color_t border);

int FillRect(pixmap_t *pixmap, int xorg, int yorg,
               int width, int height,
               brush_t *brush, color_t border);

//int PixBlit(pixblit_t* blit);

/********          Hardware accelerated          *********/

typedef struct
{
  pixmap_t  *dstpix;

  struct {
    int x0;
    int y0;
  };
  union {
    struct {
      int x1;
      int y1;
    };
    struct {
      int w;
      int h;
    };
  };
  color_t color;
  color_t border;
}draw_t;

typedef struct
{
  pixmap_t  *dstpix;

  int x;
  int y;
  int w;
  int h;

  color_t bkcolor;
  color_t fcolor;

  u32_t   bmp0;
  u32_t   bmp1;
  color_t border;
}fill_t;

typedef struct
{
  int src_x;
  int src_y;
  int dst_x;
  int dst_y;
  int w;
  int h;
}blit_t;

typedef struct
{
  pixmap_t  *dst_pixmap;
  int        dst_x;
  int        dst_y;

  pixmap_t  *src_pixmap;
  int        src_x;
  int        src_y;
  int        w;
  int        h;
}pxblit_t;

#define PX_CREATE              1
#define PX_DESTROY             2
#define PX_CLEAR               3
#define PX_DRAW_RECT           4
#define PX_FILL_RECT           5
#define PX_LINE                6
#define PX_BLIT                7
#define PX_BLIT_TRANSPARENT    8
#define PX_BLIT_ALPHA          9

/*********************************************************/

#define DBG(x) x
//  #define DBG(x)

#pragma pack (push,1)
typedef struct s_cursor
{
   u32_t   magic;                           // 'CURS'
   void    (*destroy)(struct s_cursor*);    // destructor
   u32_t   fd;                              // next object in list
   u32_t   bk;                              // prev object in list
   u32_t   pid;                             // owner id

   void    *base;                            // allocated memory
   u32_t   hot_x;                           // hotspot coords
   u32_t   hot_y;
}cursor_t;
#pragma pack (pop)


typedef struct {
    u32_t x ;
    u32_t y ;
} xPointFixed;

typedef u32_t   xFixed_16_16;

typedef xFixed_16_16  xFixed;

#define XFIXED_BITS 16

#define xFixedToInt(f)  (int) ((f) >> XFIXED_BITS)
#define IntToxFixed(i)  ((xFixed) (i) << XFIXED_BITS)

#define xFixedToFloat(f) (((float) (f)) / 65536)

#define PICT_FORMAT(bpp,type,a,r,g,b) (((bpp) << 24) |  \
					 ((type) << 16) | \
					 ((a) << 12) | \
					 ((r) << 8) | \
					 ((g) << 4) | \
					 ((b)))

#define PICT_FORMAT_A(f)  (((f) >> 12) & 0x0f)
#define PICT_FORMAT_RGB(f)  (((f)      ) & 0xfff)

#define PICT_TYPE_OTHER 0
#define PICT_TYPE_A     1
#define PICT_TYPE_ARGB	2
#define PICT_TYPE_ABGR	3
#define PICT_TYPE_COLOR	4
#define PICT_TYPE_GRAY	5

typedef enum _PictFormatShort {
   PICT_a8r8g8b8 =	PICT_FORMAT(32,PICT_TYPE_ARGB,8,8,8,8),
   PICT_x8r8g8b8 =	PICT_FORMAT(32,PICT_TYPE_ARGB,0,8,8,8),
   PICT_a8b8g8r8 =	PICT_FORMAT(32,PICT_TYPE_ABGR,8,8,8,8),
   PICT_x8b8g8r8 =	PICT_FORMAT(32,PICT_TYPE_ABGR,0,8,8,8),

/* 24bpp formats */
   PICT_r8g8b8 =	PICT_FORMAT(24,PICT_TYPE_ARGB,0,8,8,8),
   PICT_b8g8r8 =	PICT_FORMAT(24,PICT_TYPE_ABGR,0,8,8,8),

/* 16bpp formats */
   PICT_r5g6b5 =	PICT_FORMAT(16,PICT_TYPE_ARGB,0,5,6,5),
   PICT_b5g6r5 =	PICT_FORMAT(16,PICT_TYPE_ABGR,0,5,6,5),

   PICT_a1r5g5b5 =	PICT_FORMAT(16,PICT_TYPE_ARGB,1,5,5,5),
   PICT_x1r5g5b5 =	PICT_FORMAT(16,PICT_TYPE_ARGB,0,5,5,5),
   PICT_a1b5g5r5 =	PICT_FORMAT(16,PICT_TYPE_ABGR,1,5,5,5),
   PICT_x1b5g5r5 =	PICT_FORMAT(16,PICT_TYPE_ABGR,0,5,5,5),
   PICT_a4r4g4b4 =	PICT_FORMAT(16,PICT_TYPE_ARGB,4,4,4,4),
   PICT_x4r4g4b4 =	PICT_FORMAT(16,PICT_TYPE_ARGB,0,4,4,4),
   PICT_a4b4g4r4 =	PICT_FORMAT(16,PICT_TYPE_ABGR,4,4,4,4),
   PICT_x4b4g4r4 =	PICT_FORMAT(16,PICT_TYPE_ABGR,0,4,4,4),

/* 8bpp formats */
   PICT_a8 =		PICT_FORMAT(8,PICT_TYPE_A,8,0,0,0),
   PICT_r3g3b2 =	PICT_FORMAT(8,PICT_TYPE_ARGB,0,3,3,2),
   PICT_b2g3r3 =	PICT_FORMAT(8,PICT_TYPE_ABGR,0,3,3,2),
   PICT_a2r2g2b2 =	PICT_FORMAT(8,PICT_TYPE_ARGB,2,2,2,2),
   PICT_a2b2g2r2 =	PICT_FORMAT(8,PICT_TYPE_ABGR,2,2,2,2),

   PICT_c8 =		PICT_FORMAT(8,PICT_TYPE_COLOR,0,0,0,0),
   PICT_g8 =		PICT_FORMAT(8,PICT_TYPE_GRAY,0,0,0,0),

   PICT_x4a4 =		PICT_FORMAT(8,PICT_TYPE_A,4,0,0,0),

   PICT_x4c4 =		PICT_FORMAT(8,PICT_TYPE_COLOR,0,0,0,0),
   PICT_x4g4 =		PICT_FORMAT(8,PICT_TYPE_GRAY,0,0,0,0),

/* 4bpp formats */
   PICT_a4 =		PICT_FORMAT(4,PICT_TYPE_A,4,0,0,0),
   PICT_r1g2b1 =	PICT_FORMAT(4,PICT_TYPE_ARGB,0,1,2,1),
   PICT_b1g2r1 =	PICT_FORMAT(4,PICT_TYPE_ABGR,0,1,2,1),
   PICT_a1r1g1b1 =	PICT_FORMAT(4,PICT_TYPE_ARGB,1,1,1,1),
   PICT_a1b1g1r1 =	PICT_FORMAT(4,PICT_TYPE_ABGR,1,1,1,1),

   PICT_c4 =		PICT_FORMAT(4,PICT_TYPE_COLOR,0,0,0,0),
   PICT_g4 =		PICT_FORMAT(4,PICT_TYPE_GRAY,0,0,0,0),

/* 1bpp formats */
   PICT_a1 =		PICT_FORMAT(1,PICT_TYPE_A,1,0,0,0),

   PICT_g1 =		PICT_FORMAT(1,PICT_TYPE_GRAY,0,0,0,0),
} PictFormatShort;

void dump_mem();




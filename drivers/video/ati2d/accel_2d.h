
#define PX_CREATE              1
#define PX_DESTROY             2
#define PX_CLEAR               3
#define PX_DRAW_RECT           4
#define PX_FILL_RECT           5
#define PX_LINE                6
#define PX_BLIT                7
#define PX_BLIT_TRANSPARENT    8
#define PX_BLIT_ALPHA          9



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

typedef struct
{
  unsigned   width;
  unsigned   height;
  u32_t      format;
  u32_t      flags;
  size_t     pitch;
  void      *mapped;

  u32_t     handle;
}pixmap_t;


typedef struct
{
  unsigned   width;
  unsigned   height;
  u32_t      format;
  u32_t      flags;
  size_t     pitch;
  void      *mapped;

  unsigned  pitch_offset;
  addr_t    local;
}local_pixmap_t;

#define  PX_MEM_SYSTEM    0
#define  PX_MEM_LOCAL     1
#define  PX_MEM_GART      2

#define  PX_MEM_MASK      3

#define  PX_LOCK          1

typedef struct
{
  local_pixmap_t  *dstpix;

  color_t color;
}io_clear_t;

typedef struct
{
  local_pixmap_t  *dstpix;

  struct
  {
    int x0;
    int y0;
  };
  union
  {
    struct
    {
      int x1;
      int y1;
    };
    struct
    {
      int w;
      int h;
    };
  };
  color_t color;
  color_t border;
}io_draw_t;

typedef struct
{
  local_pixmap_t  *dstpix;

  int x;
  int y;
  int w;
  int h;

  color_t bkcolor;
  color_t fcolor;

  u32_t   bmp0;
  u32_t   bmp1;
  color_t border;
}io_fill_t;

typedef struct
{
  local_pixmap_t  *dstpix;
  int        dst_x;
  int        dst_y;

  local_pixmap_t  *srcpix;
  int        src_x;
  int        src_y;
  int        w;
  int        h;

  union {
    color_t    key;
    color_t    alpha;
  };
}io_blit_t;


static addr_t bind_pixmap(local_pixmap_t *pixmap);


int CreatePixmap(pixmap_t *io);

int DestroyPixmap(pixmap_t *io);

int ClearPixmap(io_clear_t *io);

int Line(io_draw_t *draw);

int DrawRect(io_draw_t * draw);

int FillRect(io_fill_t * fill);

int Blit(io_blit_t* blit);

int BlitTransparent(io_blit_t* blit);





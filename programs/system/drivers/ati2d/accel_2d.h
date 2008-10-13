
#define PX_CREATE              1
#define PX_DESTROY             2
#define PX_DRAW_RECT           3
#define PX_FILL_RECT           4
#define PX_LINE                5
#define PX_BLIT                6
#define PX_BLIT_TRANSPARENT    7
#define PX_BLIT_ALPHA          8

//#define BLIT         4
//#define COMPIZ       5


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
  unsigned  width;
  unsigned  height;
  u32_t     format;
  u32_t     flags;
  unsigned  pitch;
  void      *mapped;
  u32_t     handle;
}pixmap_t;


typedef struct
{
  unsigned  width;
  unsigned  height;
  u32_t     format;
  u32_t     flags;

  unsigned  pitch;
  void      *mapped;

  unsigned  pitch_offset;
  void      *local;
}local_pixmap_t;

//int CreatePixmap(userpixmap_t *io);
//int DestroyPixmap(userpixmap_t *io);
//int LockPixmap(userpixmap_t *io);
//int UnlockPixmap(userpixmap_t *io);

#define   PX_LOCK            1

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
}draw_t;

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
  local_pixmap_t  *dstpix;
  int        dst_x;
  int        dst_y;

  local_pixmap_t  *srcpix;
  int        src_x;
  int        src_y;
  int        w;
  int        h;
}pixblit_t;


int Line2P(draw_t *draw);

int DrawRect(draw_t * draw);
int FillRect(fill_t * fill);

int Blit(blit_t *blit);

int RadeonComposite( blit_t *blit);


int PixBlit(pixblit_t* blit);





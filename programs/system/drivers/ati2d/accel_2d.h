
#define FILL_RECT  1
#define DRAW_RECT  2
#define LINE_2P    3
#define BLIT       4
#define COMPIZ     5
#define PIXMAP     6
#define PIXBLIT    7
#define PIXLOCK    8


typedef unsigned int color_t;

typedef struct
{
  pixmap_t  *dstpix;

  int     x;
  int     y;
  u32_t   w;
  u32_t   h;
  color_t color;
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
  int x0;
  int y0;
  int x1;
  int y1;
  u32 color;
}line2p_t;

typedef struct
{
  pixmap_t  *pixmap;
  void      *usermap;
  u32_t      format;
  u32_t      pitch;

  u32_t      width;
  u32_t      height;
}userpixmap_t;

typedef struct
{
  pixmap_t  *dstpix;
  int        dst_x;
  int        dst_y;

  pixmap_t  *srcpix;
  int        src_x;
  int        src_y;
  int        w;
  int        h;
}pixblit_t;


int LineClip( int *x1, int *y1, int *x2, int *y2 );
int BlockClip( int *x1, int *y1, int *x2, int* y2);

int DrawRect(draw_t * draw);
int FillRect(fill_t * fill);

int Line2P(line2p_t *draw);

int Blit(blit_t *blit);

int RadeonComposite( blit_t *blit);

int CreatePixmap(userpixmap_t *io);

int PixBlit(pixblit_t* blit);

int LockPixmap(userpixmap_t *io);

# define RADEON_GMC_SRC_PITCH_OFFSET_CNTL (1 << 0)
#	define RADEON_GMC_DST_PITCH_OFFSET_CNTL	(1 << 1)
# define RADEON_GMC_BRUSH_SOLID_COLOR     (13 << 4)
# define RADEON_GMC_BRUSH_NONE            (15 << 4)
# define RADEON_GMC_DST_16BPP             (4 << 8)
# define RADEON_GMC_DST_24BPP             (5 << 8)
# define RADEON_GMC_DST_32BPP             (6 << 8)
# define RADEON_GMC_DST_DATATYPE_SHIFT     8
# define RADEON_GMC_SRC_DATATYPE_COLOR    (3 << 12)
# define RADEON_DP_SRC_SOURCE_MEMORY      (2 << 24)
# define RADEON_DP_SRC_SOURCE_HOST_DATA   (3 << 24)
# define RADEON_GMC_CLR_CMP_CNTL_DIS      (1 << 28)
# define RADEON_GMC_WR_MSK_DIS            (1 << 30)
# define RADEON_ROP3_S                 0x00cc0000
# define RADEON_ROP3_P                 0x00f00000

#define RADEON_CP_PACKET0              0x00000000
#define RADEON_CP_PACKET1              0x40000000
#define RADEON_CP_PACKET2              0x80000000
#define RADEON_CP_PACKET3              0xC0000000

# define RADEON_CNTL_PAINT             0x00009100
# define RADEON_CNTL_BITBLT            0x00009200

# define RADEON_CNTL_PAINT_POLYLINE    0x00009500
# define RADEON_CNTL_PAINT_MULTI       0x00009A00

#define CP_PACKET0(reg, n)            \
	(RADEON_CP_PACKET0 | ((n) << 16) | ((reg) >> 2))

#define CP_PACKET1(reg0, reg1)            \
	(RADEON_CP_PACKET1 | (((reg1) >> 2) << 11) | ((reg0) >> 2))

#define CP_PACKET2()              \
  (RADEON_CP_PACKET2)

#define CP_PACKET3( pkt, n )            \
	(RADEON_CP_PACKET3 | (pkt) | ((n) << 16))

#define BEGIN_RING( n ) do {            \
  ring = rhd.ring_base;                 \
  write = rhd.ring_wp;                  \
} while (0)

#define ADVANCE_RING()

#define OUT_RING( x ) do {        \
	ring[write++] = (x);						\
} while (0)

#define OUT_RING_REG(reg, val)            \
do {									\
    OUT_RING(CP_PACKET0(reg, 0));					\
    OUT_RING(val);							\
} while (0)

#define DRM_MEMORYBARRIER()  __asm volatile("lock; addl $0,0(%%esp)" : : : "memory");

#define COMMIT_RING() do {                            \
  rhd.ring_wp = write & 0x1FFF;                       \
  /* Flush writes to ring */                          \
  DRM_MEMORYBARRIER();                                \
  /*GET_RING_HEAD( dev_priv );          */            \
  OUTREG( RADEON_CP_RB_WPTR, rhd.ring_wp);            \
	/* read from PCI bus to ensure correct posting */		\
  INREG( RADEON_CP_RB_RPTR );                         \
} while (0)


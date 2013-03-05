
#include <stdio.h>
#include <pixlib2.h>
#include "system.h"


#define DISPLAY_VERSION     0x0200     /*      2.00     */

#define SRV_GETVERSION       0
#define SRV_GET_CAPS         3

#define SRV_CREATE_SURFACE  10
#define SRV_DESTROY_SURFACE     11
#define SRV_LOCK_SURFACE    12
#define SRV_UNLOCK_SURFACE      13
#define SRV_RESIZE_SURFACE      14
#define SRV_BLIT_BITMAP         15
#define SRV_BLIT_TEXTURE        16
#define SRV_BLIT_VIDEO          17


#define BUFFER_SIZE(n) ((n)*sizeof(uint32_t))
#define __ALIGN_MASK(x,mask)  (((x)+(mask))&~(mask))
#define ALIGN(x,a)            __ALIGN_MASK(x,(typeof(x))(a)-1)

int sna_init(uint32_t service);
void sna_fini();    

int  sna_create_bitmap(bitmap_t *bitmap);
void sna_destroy_bitmap(bitmap_t *bitmap);
void sna_lock_bitmap(bitmap_t *bitmap);
int  sna_blit_copy(bitmap_t *src_bitmap, int dst_x, int dst_y,
                  int w, int h, int src_x, int src_y);
int  sna_blit_tex(bitmap_t *src_bitmap, int dst_x, int dst_y,
                  int w, int h, int src_x, int src_y);


static uint32_t service;
static uint32_t blit_caps;
static uint32_t screen_width;
static uint32_t screen_height;

typedef struct
{
  unsigned      handle;
  unsigned      io_code;
  void          *input;
  int           inp_size;
  void          *output;
  int           out_size;
}ioctl_t;


typedef struct
{
    uint32_t  idx;
    union
    {
        uint32_t opt[2];
        struct {
            uint32_t max_tex_width;
            uint32_t max_tex_height;
        }cap1;
    };
}hwcaps_t;

static uint32_t get_service(char *name)
{
  uint32_t retval = 0;
  asm volatile ("int $0x40"
      :"=a"(retval)
      :"a"(68),"b"(16),"c"(name)
      :"memory");

  return retval;
};

static int call_service(ioctl_t *io)
{
  int retval;

  asm volatile("int $0x40"
      :"=a"(retval)
      :"a"(68),"b"(17),"c"(io)
      :"memory","cc");

  return retval;
};


uint32_t init_pixlib(uint32_t caps)
{
    uint32_t  api_version;
    uint32_t  screensize;
    hwcaps_t  hwcaps;
    ioctl_t   io;

 //   __asm__ __volatile__("int3");

    screensize    = GetScreenSize();
    screen_width  = screensize >> 16;
    screen_height = screensize & 0xFFFF;

    service = get_service("DISPLAY");
    if(service == 0)
        goto fail;

    io.handle   = service;
    io.io_code  = SRV_GETVERSION;
    io.input    = NULL;
    io.inp_size = 0;
    io.output   = &api_version;
    io.out_size = BUFFER_SIZE(1);

    if (call_service(&io)!=0)
        goto fail;

    if( (DISPLAY_VERSION > (api_version & 0xFFFF)) ||
        (DISPLAY_VERSION < (api_version >> 16)))
        goto fail;

#if 0
/*
 * Let's see what this service can do
*/
    hwcaps.idx  = 0;

    io.handle   = service;
    io.io_code  = SRV_GET_CAPS;
    io.input    = &hwcaps;
    io.inp_size = sizeof(hwcaps);
    io.output   = NULL;
    io.out_size = 0;

    if (call_service(&io)!=0)
        goto fail;

    blit_caps = hwcaps.opt[0];

    printf("\nDISPLAY service handle %x\n", service);

    if( blit_caps )
        printf("service caps %s%s%s\n",
               (blit_caps & HW_BIT_BLIT) != 0 ?"HW_BIT_BLIT ":"",
               (blit_caps & HW_TEX_BLIT) != 0 ?"HW_TEX_BLIT ":"",
               (blit_caps & HW_VID_BLIT) != 0 ?"HW_VID_BLIT ":"");
#endif

    blit_caps = caps & sna_init(service);

    if( blit_caps )
        printf("service caps %s%s%s\n",
               (blit_caps & HW_BIT_BLIT) != 0 ?"HW_BIT_BLIT ":"",
               (blit_caps & HW_TEX_BLIT) != 0 ?"HW_TEX_BLIT ":"",
               (blit_caps & HW_VID_BLIT) != 0 ?"HW_VID_BLIT ":"");
    
    return blit_caps;

fail:
    service = 0;
    return 0;
};

void done_pixlib()
{
    
    sna_fini();    
    
};


int create_bitmap(bitmap_t *bitmap)
{
 //    __asm__ __volatile__("int3");

    uint32_t   size;
    uint32_t   pitch;
    uint8_t   *buffer;

    if( bitmap->flags & (HW_BIT_BLIT | HW_TEX_BLIT ))
        return sna_create_bitmap(bitmap);

//    if( bitmap->flags &&  blit_caps & HW_BIT_BLIT )
//        return sna_create_bitmap(bitmap);

    pitch = ALIGN(bitmap->width*4, 16);
    size  = pitch * bitmap->height;

    buffer = (uint8_t*)user_alloc(size);
    if( buffer )
    {
        bitmap->handle = 0;
        bitmap->pitch  = pitch;
        bitmap->data   = buffer;
        bitmap->flags  = 0;
        return 0;
    };

    printf("Cannot alloc frame buffer\n\r");

    return -1;
    
};

int destroy_bitmap(bitmap_t *bitmap)
{
    if( bitmap->flags & (HW_BIT_BLIT | HW_TEX_BLIT ))
        sna_destroy_bitmap(bitmap);
    return 0;    
};

int lock_bitmap(bitmap_t *bitmap)
{
 //    __asm__ __volatile__("int3");

    if( bitmap->flags & (HW_BIT_BLIT | HW_TEX_BLIT ))
        sna_lock_bitmap(bitmap);

    return 0;
};

int blit_bitmap(bitmap_t *bitmap, int dst_x, int dst_y,
                int w, int h)
{
    int err;

    if( bitmap->flags & (HW_BIT_BLIT | HW_TEX_BLIT ) )
        return sna_blit_tex(bitmap, dst_x, dst_y, w, h, 0, 0);


    struct blit_call bc;

    bc.dstx = dst_x;
    bc.dsty = dst_y;
    bc.w    = w;
    bc.h    = h;
    bc.srcx = 0;
    bc.srcy = 0;
    bc.srcw = w;
    bc.srch = h;
    bc.stride = bitmap->pitch;
    bc.bitmap = bitmap->data;

    __asm__ __volatile__(
    "int $0x40"
    :"=a"(err)
    :"a"(73),"b"(0x00),"c"(&bc)
    :"memory");
    
    return err;
};

int resize_bitmap(bitmap_t *bitmap)
{
 //    __asm__ __volatile__("int3");

    if( bitmap->flags && blit_caps & HW_BIT_BLIT )
    {
        struct __attribute__((packed))
        {
            uint32_t  handle;
            char     *data;
            uint32_t  new_w;
            uint32_t  new_h;
            uint32_t  pitch;
        }io_14;

        ioctl_t io;
        int     err;

        io_14.handle = bitmap->handle;
        io_14.new_w  = bitmap->width;
        io_14.new_h  = bitmap->height;

        io.handle    = service;
        io.io_code   = SRV_RESIZE_SURFACE;
        io.input     = &io_14;
        io.inp_size  = BUFFER_SIZE(5);
        io.output    = NULL;
        io.out_size  = 0;

        err = call_service(&io);
        if(err==0)
    {
            bitmap->pitch  = io_14.pitch;
            bitmap->data   = io_14.data;
        };
        return err;
    };

    uint32_t   size;
    uint32_t   pitch;
    uint8_t   *buffer;

    pitch = ALIGN(bitmap->width*4, 16);
    size  = pitch * bitmap->height;

    buffer = (uint8_t*)user_realloc(bitmap->data, size);
    if( buffer )
    {
        bitmap->handle = 0;
        bitmap->pitch  = pitch;
        bitmap->data   = buffer;
        return 0;
    };

    printf("Cannot realloc frame buffer\n\r");

    return -1;
};


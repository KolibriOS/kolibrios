#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <stdio.h>
#include <fcntl.h>
#include "../winlib/winlib.h"
#include "fplay.h"

#define DISPLAY_VERSION     0x0200     /*      2.00     */

#define SRV_GETVERSION       0
#define SRV_GET_CAPS         3

#define SRV_CREATE_SURFACE  10
#define SRV_LOCK_SURFACE    12

#define SRV_BLIT_VIDEO      20

#define __ALIGN_MASK(x,mask)  (((x)+(mask))&~(mask))
#define ALIGN(x,a)            __ALIGN_MASK(x,(typeof(x))(a)-1)

//void InitPixlib(uint32_t flags);

static uint32_t service;
static uint32_t blit_caps;

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

#define BUFFER_SIZE(n) ((n)*sizeof(uint32_t))



uint32_t InitPixlib(uint32_t caps)
{
    uint32_t  api_version;
    hwcaps_t  hwcaps;
    ioctl_t   io;

 //   __asm__ __volatile__("int3");

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

    blit_caps&= caps;
    return blit_caps;

fail:
    service = 0;
    return 0;
};


int create_bitmap(bitmap_t *bitmap)
{
 //    __asm__ __volatile__("int3");

    if( blit_caps & HW_BIT_BLIT )
    {
        struct __attribute__((packed))  /*     SRV_CREATE_SURFACE    */
        {
            uint32_t  handle;           // ignored
            void      *data;            // ignored

            uint32_t  width;
            uint32_t  height;
            uint32_t  pitch;            // ignored

            uint32_t  max_width;
            uint32_t  max_height;
            uint32_t  format;           // reserved mbz
        }io_10;

        ioctl_t io;
        int     err;

//        printf("create bitmap %d x %d\n",
//                bitmap->width, bitmap->height);

        io_10.width      = bitmap->width;
        io_10.height     = bitmap->height;
        io_10.max_width  = 0;
        io_10.max_height = 0;
        io_10.format     = 0;

        io.handle   = service;
        io.io_code  = SRV_CREATE_SURFACE;
        io.input    = &io_10;
        io.inp_size = BUFFER_SIZE(8);
        io.output   = NULL;
        io.out_size = 0;

        err = call_service(&io);
        if(err==0)
        {
            bitmap->handle = io_10.handle;
            bitmap->pitch  = io_10.pitch;
            bitmap->data   = io_10.data;
//            printf("Create hardware surface %x pitch %d, buffer %x\n",
//                     bitmap->handle, bitmap->pitch, bitmap->data);
            return 0;
        };
        return err;
    };

    uint32_t   size;
    uint32_t   pitch;
    uint8_t   *buffer;

    pitch = ALIGN(bitmap->width*4, 16);
    size  = pitch * bitmap->height;

    buffer = (uint8_t*)user_alloc(size);
    if( buffer )
    {
        bitmap->handle = 0;
        bitmap->pitch  = pitch;
        bitmap->data   = buffer;
        return 0;
    };

    printf("Cannot alloc frame buffer\n\r");

    return -1;
};

int lock_bitmap(bitmap_t *bitmap)
{
 //    __asm__ __volatile__("int3");

    if( blit_caps & HW_BIT_BLIT )
    {
        struct __attribute__((packed))  /*     SRV_CREATE_SURFACE    */
        {
            uint32_t  handle;           // ignored
            void      *data;            // ignored

            uint32_t  width;
            uint32_t  height;
            uint32_t  pitch;            // ignored

        }io_12;

        ioctl_t io;
        int     err;

        io_12.handle  = bitmap->handle;
        io_12.pitch   = 0;
        io_12.data    = 0;

        io.handle   = service;
        io.io_code  = SRV_LOCK_SURFACE;
        io.input    = &io_12;
        io.inp_size = BUFFER_SIZE(5);
        io.output   = NULL;
        io.out_size = 0;

        err = call_service(&io);
        if(err==0)
        {
            bitmap->pitch  = io_12.pitch;
            bitmap->data   = io_12.data;
//            printf("Lock hardware surface %x pitch %d, buffer %x\n",
//                     bitmap->handle, bitmap->pitch, bitmap->data);
            return 0;
        };
        return err;
    };

    return 0;
};

struct blit_call
{
    int dstx;
    int dsty;
    int w;
    int h;

    int srcx;
    int srcy;
    int srcw;
    int srch;

    unsigned char *bitmap;
    int   stride;
};

int blit_bitmap(bitmap_t *bitmap, int dst_x, int dst_y,
                int w, int h)
{

    if( blit_caps & HW_BIT_BLIT )
    {

/*
 *   Now you will experience the full power of the dark side...
*/

        struct __attribute__((packed))
        {
            uint32_t handle;
            int      dst_x;
            int      dst_y;
            int      src_x;
            int      src_y;
            uint32_t w;
            uint32_t h;
        }io_20;

        ioctl_t io;
        int     err;

        io_20.handle = bitmap->handle;
        io_20.dst_x  = dst_x;
        io_20.dst_y  = dst_y;
        io_20.src_x  = 0;
        io_20.src_y  = 0;
        io_20.w      = w;
        io_20.h      = h;

        io.handle    = service;
        io.io_code   = SRV_BLIT_VIDEO;
        io.input     = &io_20;
        io.inp_size  = BUFFER_SIZE(7);
        io.output    = NULL;
        io.out_size  = 0;

//        printf("do blit %x pitch %d\n",bitmap->handle,
//                bitmap->pitch);
        err = call_service(&io);
        if (call_service(&io)==0)
        {
            //bitmap->data = NULL;    Not now, Serge
//            printf("blit done\n");
            delay(1);
            return 0;
        };
        return err;
    };

    volatile struct blit_call bc;

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
    ::"a"(73),"b"(0),"c"(&bc));
    
    return 0;
};


static inline void* user_realloc(void *mem, size_t size)
{
    void *val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(68),"b"(12),"c"(size),"d"(mem)
    :"memory");

    return val;
}

int resize_bitmap(bitmap_t *bitmap)
{
 //    __asm__ __volatile__("int3");

    if( blit_caps & HW_BIT_BLIT )
    {
       /*   work in progress   */
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


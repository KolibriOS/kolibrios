#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include <kos32sys.h>

#include "pixlib3.h"
#include "pixdriver.h"

#define DISPLAY_VERSION         0x0200  /*      2.00     */

#define SRV_GETVERSION              0
#define SRV_GET_CAPS                3


#define BUFFER_SIZE(n) ((n)*sizeof(uint32_t))
#define __ALIGN_MASK(x,mask)  (((x)+(mask))&~(mask))
#define ALIGN(x,a)            __ALIGN_MASK(x,(typeof(x))(a)-1)

struct bitmap
{
    uint32_t    width;
    uint32_t    height;
    uint32_t    pitch;
    void       *buffer;
    uint32_t    size;
};

static uint32_t fd;
static struct pix_driver *driver;


static bitmap_t *sw_create_bitmap(uint32_t width, uint32_t height)
{
    bitmap_t *bitmap;

    bitmap = malloc(sizeof(bitmap_t));
    if(bitmap == NULL)
        goto err_0;

    bitmap->width  = width;
    bitmap->height = height;

    bitmap->pitch = ALIGN(width * 4, 16);
    bitmap->size = ALIGN(bitmap->pitch * height, 4096);

    bitmap->buffer = user_alloc(bitmap->size);
    if (bitmap->buffer == NULL)
        goto err_1;

    return bitmap;

err_1:
    free(bitmap);
err_0:
    return NULL;
};

static int sw_destroy_bitmap(bitmap_t * bitmap)
{
    user_free(bitmap->buffer);
    free(bitmap);
    return 0;
};

static void *sw_lock_bitmap(bitmap_t *bitmap, uint32_t *pitch)
{
    *pitch = bitmap->pitch;

    return bitmap->buffer;
};

static int sw_resize_bitmap(bitmap_t * bitmap, uint32_t width, uint32_t height)
{
    uint32_t size;
    uint32_t pitch;

    pitch = ALIGN(width * 4, 16);
    size = ALIGN(pitch * height, 4096);

    if (size > bitmap->size)
    {
        bitmap->buffer = user_realloc(bitmap->buffer, size);    /* grow buffer */
        if (bitmap->buffer == NULL)
            return -1;

        bitmap->size = size;
    }
    else if (size < bitmap->size)
        user_unmap(bitmap->buffer, size, bitmap->size - size); /* unmap unused pages */

    bitmap->width  = width;
    bitmap->height = height;
    bitmap->pitch  = pitch;

    return 0;
};

static int sw_blit(bitmap_t * bitmap, int dst_x, int dst_y,
                   uint32_t w, uint32_t h, int src_x, int src_y)
{
    struct blit_call bc;
    int ret;

    bc.dstx     = dst_x;
    bc.dsty     = dst_y;
    bc.w        = w;
    bc.h        = h;
    bc.srcx     = src_x;
    bc.srcy     = src_y;
    bc.srcw     = bitmap->width;
    bc.srch     = bitmap->height;
    bc.stride   = bitmap->pitch;
    bc.bitmap   = bitmap->buffer;

    __asm__ __volatile__(
    "int $0x40":"=a"(ret):"a"(73), "b"(0x00),
    "c"(&bc):"memory");

    return ret;
};

static int sw_create_client(int x, int y, uint32_t width, uint32_t height)
{
    return 0;
};

static int sw_resize_client(int x, int y, uint32_t width, uint32_t height)
{
    return 0;
};

bitmap_t *pxCreateBitmap(uint32_t width, uint32_t height)
{
    return driver->create_bitmap(width, height);
};

int pxDestroyBitmap(bitmap_t *bitmap)
{
    return driver->destroy_bitmap(bitmap);
};

void *pxLockBitmap(bitmap_t *bitmap, uint32_t *pitch)
{
    return driver->lock_bitmap(bitmap, pitch);
};

int pxResizeBitmap(bitmap_t *bitmap, uint32_t width, uint32_t height)
{
    return driver->resize_bitmap(bitmap, width, height);
};

int pxBlitBitmap(bitmap_t *bitmap, int dst_x, int dst_y,
                uint32_t w, uint32_t h, int src_x, int src_y)
{
    return driver->blit_bitmap(bitmap, dst_x, dst_y,
                               w, h, src_x, src_y);
};

planar_t* pxCreatePlanar(int name, int format,
                         uint32_t width, uint32_t height,
                         uint32_t offset0, uint32_t pitch0,
                         uint32_t offset1, uint32_t pitch1,
                         uint32_t offset2, uint32_t pitch2)
{
    if(driver->create_planar)
        return driver->create_planar(name, format, width, height,
                                     offset0, pitch0, offset1, pitch1,
                                     offset2, pitch2);
    else
        return NULL;
};

int pxDestroyPlanar(planar_t *planar)
{
    return driver->destroy_planar(planar);
};

int pxBlitPlanar(planar_t *planar, int dst_x, int dst_y,
                 uint32_t w, uint32_t h, int src_x, int src_y)
{
    if(driver->blit_planar)
        return driver->blit_planar(planar, dst_x, dst_y,
                                   w, h, src_x, src_y);
    else
        return 0;
};

int pxCreateClient(int x, int y, uint32_t width, uint32_t height)
{
    return driver->create_client(x, y, width, height);
}

int pxResizeClient(int x, int y, uint32_t width, uint32_t height)
{
    return driver->resize_client(x, y, width, height);
}


static struct pix_driver sw_driver =
{
    0,
    sw_create_bitmap,
    sw_destroy_bitmap,
    sw_lock_bitmap,
    sw_resize_bitmap,
    sw_blit,
    sw_create_client,
    sw_resize_client,
    NULL,               /* fini()               */
    NULL,               /* create_planar()      */
    NULL,               /* destroy_planar()     */
    NULL                /* blit_planar()        */
};


uint32_t pxInit(int hw)
{
    void *lib;
    struct pix_driver *(*drventry)(uint32_t service);

    uint32_t api_version;
    ioctl_t io;

    if(driver != NULL)
        return driver->driver_caps;

    driver = &sw_driver;

    if(hw == 0)
        return 0;

    if (fd != 0)
        return driver->driver_caps;

    fd = get_service("DISPLAY");
    if (fd == 0)
        goto fail;

    io.handle = fd;
    io.io_code = SRV_GETVERSION;
    io.input = NULL;
    io.inp_size = 0;
    io.output = &api_version;
    io.out_size = BUFFER_SIZE(1);

    if (call_service(&io) != 0)
        goto fail;

    if ((DISPLAY_VERSION > (api_version & 0xFFFF)) ||
        (DISPLAY_VERSION < (api_version >> 16)))
        goto fail;

    lib = load_library("pixlib-gl.dll");
    if(lib == 0)
        goto fail;

    drventry = get_proc_address(lib, "DrvInit");

    if( drventry == NULL)
        goto fail;

    driver = drventry(fd);
    if(driver == NULL)
    {
        driver = &sw_driver;
        goto fail;
    };

    if (driver->driver_caps)
        printf("2D caps %s%s%s\n",
               (driver->driver_caps & HW_BIT_BLIT) != 0 ? "HW_BIT_BLIT " : "",
               (driver->driver_caps & HW_TEX_BLIT) != 0 ? "HW_TEX_BLIT " : "",
               (driver->driver_caps & HW_VID_BLIT) != 0 ? "HW_VID_BLIT " : "");

    return driver->driver_caps;

fail:
    printf("Warning! Hardware initialization failed.\n"
           "fallback to software rendering.\n");
    fd = 0;
    return 0;
};

void pxFini()
{
    if (driver->fini)
        driver->fini();
};


/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "loader.h"

#include "adapter9.h"

#include "pipe-loader/pipe_loader.h"

#include "pipe/p_screen.h"
#include "pipe/p_state.h"

#include "target-helpers/inline_drm_helper.h"
#include "target-helpers/inline_sw_helper.h"
#include "state_tracker/drm_driver.h"

#include "d3dadapter/d3dadapter9.h"
#include "d3dadapter/drm.h"

#include "xmlconfig.h"
#include "xmlpool.h"

#include <drm.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>

#define DBG_CHANNEL DBG_ADAPTER

#define VERSION_DWORD(hi, lo) \
    ((DWORD)( \
        ((DWORD)((hi) & 0xFFFF) << 16) | \
         (DWORD)((lo) & 0xFFFF) \
    ))

const char __driConfigOptionsNine[] =
DRI_CONF_BEGIN
    DRI_CONF_SECTION_PERFORMANCE
         DRI_CONF_VBLANK_MODE(DRI_CONF_VBLANK_DEF_INTERVAL_1)
    DRI_CONF_SECTION_END
    DRI_CONF_SECTION_NINE
        DRI_CONF_NINE_THROTTLE(-2)
        DRI_CONF_NINE_THREADSUBMIT("false")
    DRI_CONF_SECTION_END
DRI_CONF_END;

/* Regarding os versions, we should not define our own as that would simply be
 * weird. Defaulting to Win2k/XP seems sane considering the origin of D3D9. The
 * driver also defaults to being a generic D3D9 driver, which of course only
 * matters if you're actually using the DDI. */
#define VERSION_HIGH    VERSION_DWORD(0x0006, 0x000E) /* winxp, d3d9 */
#define VERSION_LOW     VERSION_DWORD(0x0000, 0x0001) /* version, build */

struct d3dadapter9drm_context
{
    struct d3dadapter9_context base;
    struct pipe_loader_device *dev, *swdev;
    int fd;
};

static void
drm_destroy( struct d3dadapter9_context *ctx )
{
    struct d3dadapter9drm_context *drm = (struct d3dadapter9drm_context *)ctx;

    if (ctx->ref)
        ctx->ref->destroy(ctx->ref);
    /* because ref is a wrapper around hal, freeing ref frees hal too. */
    else if (ctx->hal)
        ctx->hal->destroy(ctx->hal);

#if !GALLIUM_STATIC_TARGETS
    if (drm->swdev)
        pipe_loader_release(&drm->swdev, 1);
    if (drm->dev)
        pipe_loader_release(&drm->dev, 1);
#endif

    close(drm->fd);
    FREE(ctx);
}

/* read a DWORD in the form 0xnnnnnnnn, which is how sysfs pci id stuff is
 * formatted. */
static INLINE DWORD
read_file_dword( const char *name )
{
    char buf[32];
    int fd, r;

    fd = open(name, O_RDONLY);
    if (fd < 0) {
        DBG("Unable to get PCI information from `%s'\n", name);
        return 0;
    }

    r = read(fd, buf, 32);
    close(fd);

    return (r > 0) ? (DWORD)strtol(buf, NULL, 0) : 0;
}

/* sysfs doesn't expose the revision as its own file, so this function grabs a
 * dword at an offset in the raw PCI header. The reason this isn't used for all
 * data is that the kernel will make corrections but not expose them in the raw
 * header bytes. */
static INLINE DWORD
read_config_dword( int fd,
                   unsigned offset )
{
    DWORD r = 0;

    if (lseek(fd, offset, SEEK_SET) != offset) { return 0; }
    if (read(fd, &r, 4) != 4) { return 0; }

    return r;
}

static INLINE void
get_bus_info( int fd,
              DWORD *vendorid,
              DWORD *deviceid,
              DWORD *subsysid,
              DWORD *revision )
{
    int vid, did;

    if (loader_get_pci_id_for_fd(fd, &vid, &did)) {
        DBG("PCI info: vendor=0x%04x, device=0x%04x\n",
            vid, did);
        *vendorid = vid;
        *deviceid = did;
        *subsysid = 0;
        *revision = 0;
    } else {
        DBG("Unable to detect card. Fake GTX 680.\n");
        *vendorid = 0x10de; /* NV GTX 680 */
        *deviceid = 0x1180;
        *subsysid = 0;
        *revision = 0;
    }
}

static INLINE void
read_descriptor( struct d3dadapter9_context *ctx,
                 int fd )
{
    D3DADAPTER_IDENTIFIER9 *drvid = &ctx->identifier;

    memset(drvid, 0, sizeof(*drvid));
    get_bus_info(fd, &drvid->VendorId, &drvid->DeviceId,
                 &drvid->SubSysId, &drvid->Revision);

    strncpy(drvid->Driver, "libd3dadapter9.so", sizeof(drvid->Driver));
    strncpy(drvid->DeviceName, ctx->hal->get_name(ctx->hal), 32);
    snprintf(drvid->Description, sizeof(drvid->Description),
             "Gallium 0.4 with %s", ctx->hal->get_vendor(ctx->hal));

    drvid->DriverVersionLowPart = VERSION_LOW;
    drvid->DriverVersionHighPart = VERSION_HIGH;

    /* To make a pseudo-real GUID we use the PCI bus data and some string */
    drvid->DeviceIdentifier.Data1 = drvid->VendorId;
    drvid->DeviceIdentifier.Data2 = drvid->DeviceId;
    drvid->DeviceIdentifier.Data3 = drvid->SubSysId;
    memcpy(drvid->DeviceIdentifier.Data4, "Gallium3D", 8);

    drvid->WHQLLevel = 1; /* This fakes WHQL validaion */

    /* XXX Fake NVIDIA binary driver on Windows.
     *
     * OS version: 4=95/98/NT4, 5=2000, 6=2000/XP, 7=Vista, 8=Win7
     */
    strncpy(drvid->Driver, "nvd3dum.dll", sizeof(drvid->Driver));
    strncpy(drvid->Description, "NVIDIA GeForce GTX 680", sizeof(drvid->Description));
    drvid->DriverVersionLowPart = VERSION_DWORD(12, 6658); /* minor, build */
    drvid->DriverVersionHighPart = VERSION_DWORD(6, 15); /* OS, major */
    drvid->SubSysId = 0;
    drvid->Revision = 0;
    drvid->DeviceIdentifier.Data1 = 0xaeb2cdd4;
    drvid->DeviceIdentifier.Data2 = 0x6e41;
    drvid->DeviceIdentifier.Data3 = 0x43ea;
    drvid->DeviceIdentifier.Data4[0] = 0x94;
    drvid->DeviceIdentifier.Data4[1] = 0x1c;
    drvid->DeviceIdentifier.Data4[2] = 0x83;
    drvid->DeviceIdentifier.Data4[3] = 0x61;
    drvid->DeviceIdentifier.Data4[4] = 0xcc;
    drvid->DeviceIdentifier.Data4[5] = 0x76;
    drvid->DeviceIdentifier.Data4[6] = 0x07;
    drvid->DeviceIdentifier.Data4[7] = 0x81;
    drvid->WHQLLevel = 0;
}

static HRESULT WINAPI
drm_create_adapter( int fd,
                    ID3DAdapter9 **ppAdapter )
{
    struct d3dadapter9drm_context *ctx = CALLOC_STRUCT(d3dadapter9drm_context);
    HRESULT hr;
    int different_device;
    const struct drm_conf_ret *throttle_ret = NULL;
    const struct drm_conf_ret *dmabuf_ret = NULL;
    driOptionCache defaultInitOptions;
    driOptionCache userInitOptions;
    int throttling_value_user = -2;

#if !GALLIUM_STATIC_TARGETS
    const char *paths[] = {
        getenv("D3D9_DRIVERS_PATH"),
        getenv("D3D9_DRIVERS_DIR"),
        PIPE_SEARCH_DIR
    };
#endif

    if (!ctx) { return E_OUTOFMEMORY; }

    ctx->base.destroy = drm_destroy;

    fd = loader_get_user_preferred_fd(fd, &different_device);
    ctx->fd = fd;
    ctx->base.linear_framebuffer = !!different_device;

#if GALLIUM_STATIC_TARGETS
    ctx->base.hal = dd_create_screen(fd);
#else
    /* use pipe-loader to dlopen appropriate drm driver */
    if (!pipe_loader_drm_probe_fd(&ctx->dev, fd, FALSE)) {
        ERR("Failed to probe drm fd %d.\n", fd);
        FREE(ctx);
        close(fd);
        return D3DERR_DRIVERINTERNALERROR;
    }

    /* use pipe-loader to create a drm screen (hal) */
    ctx->base.hal = NULL;
    for (i = 0; !ctx->base.hal && i < Elements(paths); ++i) {
        if (!paths[i]) { continue; }
        ctx->base.hal = pipe_loader_create_screen(ctx->dev, paths[i]);
    }
#endif
    if (!ctx->base.hal) {
        ERR("Unable to load requested driver.\n");
        drm_destroy(&ctx->base);
        return D3DERR_DRIVERINTERNALERROR;
    }

#if GALLIUM_STATIC_TARGETS
    dmabuf_ret = dd_configuration(DRM_CONF_SHARE_FD);
    throttle_ret = dd_configuration(DRM_CONF_THROTTLE);
#else
    dmabuf_ret = pipe_loader_configuration(ctx->dev, DRM_CONF_SHARE_FD);
    throttle_ret = pipe_loader_configuration(ctx->dev, DRM_CONF_THROTTLE);
#endif // GALLIUM_STATIC_TARGETS
    if (!dmabuf_ret || !dmabuf_ret->val.val_bool) {
        ERR("The driver is not capable of dma-buf sharing."
            "Abandon to load nine state tracker\n");
        drm_destroy(&ctx->base);
        return D3DERR_DRIVERINTERNALERROR;
    }

    if (throttle_ret && throttle_ret->val.val_int != -1) {
        ctx->base.throttling = TRUE;
        ctx->base.throttling_value = throttle_ret->val.val_int;
    } else
        ctx->base.throttling = FALSE;

    driParseOptionInfo(&defaultInitOptions, __driConfigOptionsNine);
    driParseConfigFiles(&userInitOptions, &defaultInitOptions, 0, "nine");
    if (driCheckOption(&userInitOptions, "throttle_value", DRI_INT)) {
        throttling_value_user = driQueryOptioni(&userInitOptions, "throttle_value");
        if (throttling_value_user == -1)
            ctx->base.throttling = FALSE;
        else if (throttling_value_user >= 0) {
            ctx->base.throttling = TRUE;
            ctx->base.throttling_value = throttling_value_user;
        }
    }

    if (driCheckOption(&userInitOptions, "vblank_mode", DRI_ENUM))
        ctx->base.vblank_mode = driQueryOptioni(&userInitOptions, "vblank_mode");
    else
        ctx->base.vblank_mode = 1;

    if (driCheckOption(&userInitOptions, "thread_submit", DRI_BOOL)) {
        ctx->base.thread_submit = driQueryOptionb(&userInitOptions, "thread_submit");
        if (ctx->base.thread_submit && (throttling_value_user == -2 || throttling_value_user == 0)) {
            ctx->base.throttling_value = 0;
        } else if (ctx->base.thread_submit) {
            DBG("You have set a non standard throttling value in combination with thread_submit."
                "We advise to use a throttling value of -2/0");
        }
        if (ctx->base.thread_submit && !different_device)
            DBG("You have set thread_submit but do not use a different device than the server."
                "You should not expect any benefit.");
    }

    driDestroyOptionCache(&userInitOptions);
    driDestroyOptionInfo(&defaultInitOptions);

#if GALLIUM_STATIC_TARGETS
    ctx->base.ref = ninesw_create_screen(ctx->base.hal);
#else
    /* wrap it to create a software screen that can share resources */
    if (pipe_loader_sw_probe_wrapped(&ctx->swdev, ctx->base.hal)) {
        ctx->base.ref = NULL;
        for (i = 0; !ctx->base.ref && i < Elements(paths); ++i) {
            if (!paths[i]) { continue; }
            ctx->base.ref = pipe_loader_create_screen(ctx->swdev, paths[i]);
        }
    }
#endif
    if (!ctx->base.ref) {
        ERR("Couldn't wrap drm screen to swrast screen. Software devices "
            "will be unavailable.\n");
    }

    /* read out PCI info */
    read_descriptor(&ctx->base, fd);

    /* create and return new ID3DAdapter9 */
    hr = NineAdapter9_new(&ctx->base, (struct NineAdapter9 **)ppAdapter);
    if (FAILED(hr)) {
        drm_destroy(&ctx->base);
        return hr;
    }

    return D3D_OK;
}

const struct D3DAdapter9DRM drm9_desc = {
    .major_version = D3DADAPTER9DRM_MAJOR,
    .minor_version = D3DADAPTER9DRM_MINOR,
    .create_adapter = drm_create_adapter
};

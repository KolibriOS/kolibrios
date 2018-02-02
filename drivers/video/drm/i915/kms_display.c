#include <drm/drmP.h>
#include "intel_drv.h"
#include <drm/i915_drm.h>
#include <drm/drm_plane_helper.h>
#include "i915_drv.h"

#include <syscall.h>
#include <display.h>

int printf ( const char * format, ... );

void init_system_cursors(struct drm_device *dev);

display_t *os_display;

u32 cmd_buffer;
u32 cmd_offset;

void init_render();
int  sna_init();

static char *manufacturer_name(unsigned char *x)
{
    static char name[4];

    name[0] = ((x[0] & 0x7C) >> 2) + '@';
    name[1] = ((x[0] & 0x03) << 3) + ((x[1] & 0xE0) >> 5) + '@';
    name[2] = (x[1] & 0x1F) + '@';
    name[3] = 0;

    return name;
}

static int count_connector_modes(struct drm_connector* connector)
{
    struct drm_display_mode  *mode;
    int count = 0;

    list_for_each_entry(mode, &connector->modes, head)
        count++;

    return count;
};

struct drm_framebuffer *get_framebuffer(struct drm_device *dev, struct drm_display_mode *mode, int tiling)
{
    struct drm_i915_private    *dev_priv = dev->dev_private;
    struct intel_fbdev         *ifbdev   = dev_priv->fbdev;
    struct intel_framebuffer   *intel_fb = ifbdev->fb;
    struct drm_framebuffer     *fb = &intel_fb->base;
    struct drm_i915_gem_object *obj = NULL;
    int stride, size;

    stride = mode->hdisplay *4;

    if(IS_GEN3(dev))
        tiling = 0;

    if(tiling)
    {
        int gen3size;

        stride = ALIGN(stride, 512);
        size = stride * ALIGN(mode->vdisplay, 8);
        size = ALIGN(size, 4096);
    }
    else
    {
        stride = ALIGN(stride, 64);
        size = stride * ALIGN(mode->vdisplay, 2);
    }

    DRM_DEBUG_KMS("size %x stride %x\n", size, stride);

    if(intel_fb == NULL || size > intel_fb->obj->base.size)
    {
        struct drm_mode_fb_cmd2 mode_cmd = {};
        int ret;

        DRM_DEBUG_KMS("remove old framebuffer\n");
        set_fake_framebuffer();
        drm_framebuffer_remove(fb);
        ifbdev->fb = NULL;
        fb = NULL;
        DRM_DEBUG_KMS("create new framebuffer\n");

        mode_cmd.width  = mode->hdisplay;
        mode_cmd.height = mode->vdisplay;

        mode_cmd.pitches[0] = stride;
        mode_cmd.pixel_format = DRM_FORMAT_XRGB8888;

        mutex_lock(&dev->struct_mutex);

        /* If the FB is too big, just don't use it since fbdev is not very
        * important and we should probably use that space with FBC or other
        * features. */
        if (size * 2 < dev_priv->gtt.stolen_usable_size)
            obj = i915_gem_object_create_stolen(dev, size);
        if (obj == NULL)
            obj = i915_gem_alloc_object(dev, size);
        if (!obj) {
            DRM_ERROR("failed to allocate framebuffer\n");
            ret = -ENOMEM;
            goto out;
        }

        fb = __intel_framebuffer_create(dev, &mode_cmd, obj);
        if (IS_ERR(fb)) {
            ret = PTR_ERR(fb);
            goto out_unref;
        }

        /* Flush everything out, we'll be doing GTT only from now on */
        ret = intel_pin_and_fence_fb_obj(NULL, fb, NULL);
        if (ret) {
            DRM_ERROR("failed to pin obj: %d\n", ret);
            goto out_fb;
        }
        mutex_unlock(&dev->struct_mutex);
        ifbdev->fb = to_intel_framebuffer(fb);
    }

    obj = ifbdev->fb->obj;

    if(tiling)
    {
        obj->tiling_mode = I915_TILING_X;
        fb->modifier[0]  = I915_FORMAT_MOD_X_TILED;
        obj->fence_dirty = true;
        obj->stride      = stride;
    };

    if (obj->base.name == 0)
    {
        int ret;

        mutex_lock(&dev->object_name_lock);
        idr_preload(GFP_KERNEL);
        ret = idr_alloc(&dev->object_name_idr, &obj->base, 1, 0, GFP_NOWAIT);
        idr_preload_end();
        mutex_unlock(&dev->object_name_lock);
        obj->base.name = ret;
        obj->base.handle_count++;
        DRM_DEBUG_KMS("%s allocate fb name %d\n", __FUNCTION__, obj->base.name );
    }

    fb->width  = mode->hdisplay;
    fb->height = mode->vdisplay;

    fb->pitches[0]  =
    fb->pitches[1]  =
    fb->pitches[2]  =
    fb->pitches[3]  = stride;

    fb->bits_per_pixel = 32;
    fb->depth = 24;

    return fb;

out_fb:
    drm_framebuffer_remove(fb);
out_unref:
    drm_gem_object_unreference(&obj->base);
out:
    mutex_unlock(&dev->struct_mutex);
    return NULL;
}

static int set_mode(struct drm_device *dev, struct drm_connector *connector,
                    struct drm_crtc *crtc, videomode_t *reqmode, bool strict)
{
    struct drm_i915_private *dev_priv = dev->dev_private;
    struct drm_mode_config  *config   = &dev->mode_config;
    struct drm_display_mode *mode     = NULL, *tmpmode;
    struct drm_connector    *tmpc;
    struct drm_framebuffer  *fb       = NULL;
    struct drm_mode_set     set;
    char  con_edid[128];
    int ret;

    drm_modeset_lock_all(dev);

    list_for_each_entry(tmpc, &dev->mode_config.connector_list, head)
    {
        const struct drm_connector_funcs *f = tmpc->funcs;
        if(tmpc == connector)
            continue;
        f->dpms(tmpc, DRM_MODE_DPMS_OFF);
    };

    list_for_each_entry(tmpmode, &connector->modes, head)
    {
        if( (tmpmode->hdisplay == reqmode->width)  &&
            (tmpmode->vdisplay == reqmode->height) &&
            (drm_mode_vrefresh(tmpmode) == reqmode->freq) )
        {
            mode = tmpmode;
            goto do_set;
        }
    };

    if( (mode == NULL) && (strict == false) )
    {
        list_for_each_entry(tmpmode, &connector->modes, head)
        {
            if( (tmpmode->hdisplay == reqmode->width)  &&
                (tmpmode->vdisplay == reqmode->height) )
            {
                mode = tmpmode;
                goto do_set;
            }
        };
    };

out:
    drm_modeset_unlock_all(dev);
    DRM_ERROR("%s failed\n", __FUNCTION__);
    return -1;

do_set:

    drm_modeset_unlock_all(dev);

    fb = get_framebuffer(dev, mode, 1);
    if(fb == NULL)
    {
        DRM_ERROR("%s failed\n", __FUNCTION__);
        return -1;
    };
    drm_framebuffer_reference(fb);

    drm_modeset_lock_all(dev);

    memcpy(con_edid, connector->edid_blob_ptr->data, 128);

    DRM_DEBUG_KMS("set mode %dx%d: crtc %d connector %s\n"
                  "monitor: %s model %x serial number %u\n",
                mode->hdisplay, mode->vdisplay,
                crtc->base.id, connector->name,
                manufacturer_name(con_edid + 0x08),
                (unsigned short)(con_edid[0x0A] + (con_edid[0x0B] << 8)),
                (unsigned int)(con_edid[0x0C] + (con_edid[0x0D] << 8)
                + (con_edid[0x0E] << 16) + (con_edid[0x0F] << 24)));

    drm_mode_set_crtcinfo(mode, CRTC_INTERLACE_HALVE_V);

    crtc->enabled = true;
    os_display->crtc = crtc;

    DRM_DEBUG_KMS("fb:%p %dx%dx pitch %d format %x\n",
            fb,fb->width,fb->height,fb->pitches[0],fb->pixel_format);

    set.crtc = crtc;
    set.x = 0;
    set.y = 0;
    set.mode = mode;
    set.connectors = &connector;
    set.num_connectors = 1;
    set.fb = fb;

    ret = drm_mode_set_config_internal(&set);

    if ( !ret )
    {
        struct intel_framebuffer *intel_fb = to_intel_framebuffer(fb);
        struct kos_framebuffer *kfb = intel_fb->private;
        kolibri_framebuffer_update(dev, kfb);
        DRM_DEBUG_KMS("kolibri framebuffer %p\n", kfb);

        os_display->width    = mode->hdisplay;
        os_display->height   = mode->vdisplay;
        os_display->vrefresh = drm_mode_vrefresh(mode);
        sysSetFramebuffer(intel_fb->private);
        sysSetScreen(mode->hdisplay, mode->vdisplay, fb->pitches[0]);

        os_display->connector = connector;
        os_display->crtc = connector->encoder->crtc;
        os_display->supported_modes = count_connector_modes(connector);

        crtc->cursor_x = os_display->width/2;
        crtc->cursor_y = os_display->height/2;

        os_display->select_cursor(os_display->cursor);

        DRM_DEBUG_KMS("new mode %d x %d pitch %d\n",
                       mode->hdisplay, mode->vdisplay, fb->pitches[0]);
    }
    else
        DRM_ERROR("failed to set mode %d_%d on crtc %p\n",
                   fb->width, fb->height, crtc);

    drm_framebuffer_unreference(fb);
    drm_modeset_unlock_all(dev);

    return ret;
}

static int set_mode_ex(struct drm_device *dev,
                       struct drm_connector *connector, struct drm_display_mode *mode)
{
    struct drm_i915_private *dev_priv = dev->dev_private;
    struct drm_connector    *tmpc;
    struct drm_mode_config  *config   = &dev->mode_config;
    struct drm_framebuffer  *fb       = NULL;
    struct drm_mode_set     set;
    struct drm_crtc *crtc = NULL;

    char  con_edid[128];
    int stride;
    int ret;

    fb = get_framebuffer(dev, mode, 1);
    if(fb == NULL)
    {
        DRM_ERROR("%s failed\n", __FUNCTION__);
        return -1;
    };
    drm_framebuffer_reference(fb);

    drm_modeset_lock_all(dev);

    list_for_each_entry(tmpc, &dev->mode_config.connector_list, head)
    {
        const struct drm_connector_funcs *f = tmpc->funcs;
        if(tmpc == connector)
            continue;
        f->dpms(tmpc, DRM_MODE_DPMS_OFF);
    };

    crtc = connector->encoder->crtc;

    memcpy(con_edid, connector->edid_blob_ptr->data, 128);
    DRM_DEBUG_KMS("set mode %dx%d: crtc %d connector %s\n"
                  "monitor: %s model %x serial number %u\n",
                mode->hdisplay, mode->vdisplay,
                connector->encoder->crtc->base.id, connector->name,
                manufacturer_name(con_edid + 0x08),
                (unsigned short)(con_edid[0x0A] + (con_edid[0x0B] << 8)),
                (unsigned int)(con_edid[0x0C] + (con_edid[0x0D] << 8)
                + (con_edid[0x0E] << 16) + (con_edid[0x0F] << 24)));

    drm_mode_set_crtcinfo(mode, CRTC_INTERLACE_HALVE_V);

    crtc->enabled = true;
    os_display->crtc = crtc;

    DRM_DEBUG_KMS("use framebuffer %p %dx%d pitch %d format %x\n",
            fb,fb->width,fb->height,fb->pitches[0],fb->pixel_format);

    set.crtc = crtc;
    set.x = 0;
    set.y = 0;
    set.mode = mode;
    set.connectors = &connector;
    set.num_connectors = 1;
    set.fb = fb;

    ret = drm_mode_set_config_internal(&set);
    if ( !ret )
    {
        struct intel_framebuffer *intel_fb = to_intel_framebuffer(fb);
        struct kos_framebuffer *kfb = intel_fb->private;
        kolibri_framebuffer_update(dev, kfb);
        DRM_DEBUG_KMS("kolibri framebuffer %p\n", kfb);

        os_display->width    = mode->hdisplay;
        os_display->height   = mode->vdisplay;
        os_display->vrefresh = drm_mode_vrefresh(mode);
        sysSetFramebuffer(intel_fb->private);
        sysSetScreen(mode->hdisplay, mode->vdisplay, fb->pitches[0]);

        os_display->connector = connector;
        os_display->crtc = connector->encoder->crtc;
        os_display->supported_modes = count_connector_modes(connector);

        crtc->cursor_x = os_display->width/2;
        crtc->cursor_y = os_display->height/2;

        os_display->select_cursor(os_display->cursor);

        DRM_DEBUG_KMS("new mode %d x %d pitch %d\n",
                       mode->hdisplay, mode->vdisplay, fb->pitches[0]);
    }
    else
        DRM_ERROR(" failed to set mode %d_%d on crtc %p\n",
                   fb->width, fb->height, connector->encoder->crtc);

    drm_framebuffer_unreference(fb);
    drm_modeset_unlock_all(dev);
    return ret;
}

static int set_cmdline_mode(struct drm_device *dev, struct drm_connector *connector)
{
    struct drm_display_mode *mode;
    int retval;

    mode = drm_mode_create_from_cmdline_mode(dev, &connector->cmdline_mode);
    if(mode == NULL)
        return EINVAL;

    retval = set_mode_ex(dev, connector, mode);

    drm_mode_destroy(dev, mode);
    return retval;
};

static struct drm_crtc *get_possible_crtc(struct drm_device *dev, struct drm_encoder *encoder)
{
    struct drm_crtc *tmp_crtc;
    int crtc_mask = 1;

    list_for_each_entry(tmp_crtc, &dev->mode_config.crtc_list, head)
    {
        if (encoder->possible_crtcs & crtc_mask)
        {
            encoder->crtc = tmp_crtc;
            DRM_DEBUG_KMS("use CRTC %p ID %d\n", tmp_crtc, tmp_crtc->base.id);
            return tmp_crtc;
        };
        crtc_mask <<= 1;
    };
    return NULL;
};

static int check_connector(struct drm_device *dev, struct drm_connector *connector)
{
    const struct drm_connector_helper_funcs *connector_funcs;
    struct drm_encoder   *encoder;
    struct drm_crtc      *crtc;

    if( connector->status != connector_status_connected)
        return -EINVAL;

    encoder = connector->encoder;

    if(encoder == NULL)
    {
        connector_funcs = connector->helper_private;
        encoder = connector_funcs->best_encoder(connector);

        if( encoder == NULL)
        {
            DRM_DEBUG_KMS("CONNECTOR %s ID: %d no active encoders\n",
            connector->name, connector->base.id);
            return -EINVAL;
        };
        connector->encoder = encoder;
    }

    crtc = encoder->crtc;
    if(crtc == NULL)
        crtc = get_possible_crtc(dev, encoder);

    if(crtc != NULL)
    {
        DRM_DEBUG_KMS("%s connector: %p encode: %p crtc: %p\n",__FUNCTION__,
               connector, encoder, crtc);
        return 0;
    }
    else
        DRM_DEBUG_KMS("No CRTC for encoder %d\n", encoder->base.id);
    return -EINVAL;
}

static struct drm_connector* get_cmdline_connector(struct drm_device *dev, const char *cmdline)
{
    struct drm_connector *connector;

    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
    {
        int name_len = __builtin_strlen(connector->name);

        if (name_len == 0)
            continue;

        if (__builtin_strncmp(connector->name, cmdline, name_len))
            continue;

        if(check_connector(dev, connector) == 0)
            return connector;
    }
    return NULL;
}


static int choose_config(struct drm_device *dev, struct drm_connector **boot_connector,
                  struct drm_crtc **boot_crtc)
{
    struct drm_connector *connector;

    if((i915.cmdline_mode != NULL) && (*i915.cmdline_mode != 0))
    {
        connector = get_cmdline_connector(dev, i915.cmdline_mode);
        if(connector != NULL)
        {
            *boot_connector = connector;
            *boot_crtc = connector->encoder->crtc;
            return 0;
        }
    }

    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
    {
        if(check_connector(dev, connector) == 0)
        {
            *boot_connector = connector;
            *boot_crtc = connector->encoder->crtc;
            return 0;
        };
    };

    return -ENOENT;
};


static int get_boot_mode(struct drm_connector *connector, videomode_t *usermode)
{
    struct drm_display_mode *mode;

    list_for_each_entry(mode, &connector->modes, head)
    {
        if( os_display->width  == mode->hdisplay &&
            os_display->height == mode->vdisplay &&
            drm_mode_vrefresh(mode) == 60)
        {
            usermode->width  = os_display->width;
            usermode->height = os_display->height;
            usermode->freq   = 60;
            return 1;
        }
    }
    return 0;
}

int init_display_kms(struct drm_device *dev, videomode_t *usermode)
{
    struct drm_connector_helper_funcs *connector_funcs;
    struct drm_connector    *connector = NULL;
    struct drm_crtc         *crtc = NULL;
    struct drm_plane *plane;

    int ret;

    drm_for_each_plane(plane, dev)
    {
        drm_plane_helper_disable(plane);
    };

    mutex_lock(&dev->mode_config.mutex);
    ret = choose_config(dev, &connector, &crtc);
    if(ret)
    {
        mutex_unlock(&dev->mode_config.mutex);
        DRM_DEBUG_KMS("No active connectors!\n");
        return -1;
    };

    os_display = GetDisplay();
    os_display->ddev = dev;
    os_display->connector = connector;
    os_display->crtc = crtc;
    os_display->supported_modes = count_connector_modes(connector);
    mutex_unlock(&dev->mode_config.mutex);

    init_system_cursors(dev);

    ret = -1;

    if(connector->cmdline_mode.specified == true)
        ret = set_cmdline_mode(dev, connector);

    if(ret !=0)
    {
        mutex_lock(&dev->mode_config.mutex);

        if( (usermode->width == 0) ||
            (usermode->height == 0))
        {
            if( !get_boot_mode(connector, usermode))
            {
                struct drm_display_mode *mode;

                mode = list_entry(connector->modes.next, typeof(*mode), head);
                usermode->width  = mode->hdisplay;
                usermode->height = mode->vdisplay;
                usermode->freq   = drm_mode_vrefresh(mode);
            };
        };
        mutex_unlock(&dev->mode_config.mutex);

        set_mode(dev, os_display->connector, os_display->crtc, usermode, false);
    };

    return ret;
};


int set_cmdline_mode_ext(struct drm_device *dev, const char *cmdline)
{
    struct drm_connector_helper_funcs *connector_funcs;
    struct drm_connector    *connector;
    struct drm_cmdline_mode cmd_mode = {0};
    struct drm_display_mode *mode;
    char *mode_option;
    int retval = 0;
    char  con_edid[128];

    if((cmdline == NULL) || (*cmdline == 0))
        return EINVAL;

    mutex_lock(&dev->mode_config.mutex);
    connector = get_cmdline_connector(dev, cmdline);
    mutex_unlock(&dev->mode_config.mutex);

    if(connector == NULL)
        return EINVAL;

    mode_option = __builtin_strchr(cmdline,':');
    if(mode_option == NULL)
        return EINVAL;

    mode_option++;

    if( !drm_mode_parse_command_line_for_connector(mode_option, connector, &cmd_mode))
        return EINVAL;

    DRM_DEBUG_KMS("cmdline mode for connector %s %dx%d@%dHz%s%s%s\n",
                   connector->name,
                   cmd_mode.xres, cmd_mode.yres,
                   cmd_mode.refresh_specified ? cmd_mode.refresh : 60,
                   cmd_mode.rb ? " reduced blanking" : "",
                   cmd_mode.margins ? " with margins" : "",
                   cmd_mode.interlace ?  " interlaced" : "");

    mode = drm_mode_create_from_cmdline_mode(dev, &cmd_mode);
    if(mode == NULL)
        return EINVAL;

    memcpy(con_edid, connector->edid_blob_ptr->data, 128);
    DRM_DEBUG_KMS("connector: %s monitor: %s model %x serial number %u\n",
            connector->name,
            manufacturer_name(con_edid + 0x08),
            (unsigned short)(con_edid[0x0A] + (con_edid[0x0B] << 8)),
            (unsigned int)(con_edid[0x0C] + (con_edid[0x0D] << 8)
            + (con_edid[0x0E] << 16) + (con_edid[0x0F] << 24)));

    retval = set_mode_ex(dev, connector, mode);

    drm_mode_destroy(dev, mode);

    return retval;
}

void list_connectors(struct drm_device *dev)
{
    struct drm_connector *connector;
    char  con_edid[128];

    mutex_lock(&dev->mode_config.mutex);
    list_for_each_entry(connector, &dev->mode_config.connector_list, head)
    {
        if( connector->status != connector_status_connected)
            continue;

        memcpy(con_edid, connector->edid_blob_ptr->data, 128);

        if(connector ==  os_display->connector)
        {
            printf("%s mode %dx%d connected %s model %x serial number %u\n",
                   connector->name, os_display->width, os_display->height,
                   manufacturer_name(con_edid + 0x08),
                   (unsigned short)(con_edid[0x0A] + (con_edid[0x0B] << 8)),
                   (unsigned int)(con_edid[0x0C] + (con_edid[0x0D] << 8)
                   + (con_edid[0x0E] << 16) + (con_edid[0x0F] << 24)));
            continue;
        }
        else
        {
            printf("%s connected: %s model %x serial number %u\n",
                connector->name, manufacturer_name(con_edid + 0x08),
                (unsigned short)(con_edid[0x0A] + (con_edid[0x0B] << 8)),
                (unsigned int)(con_edid[0x0C] + (con_edid[0x0D] << 8)
                + (con_edid[0x0E] << 16) + (con_edid[0x0F] << 24)));
        }
    };
    mutex_unlock(&dev->mode_config.mutex);
}

int list_connector_modes(struct drm_device *dev, const char* name)
{
    struct drm_connector *connector;
    struct drm_display_mode  *drmmode;

    mutex_lock(&dev->mode_config.mutex);

    connector = get_cmdline_connector(dev, name);
    if(connector == NULL)
    {
        mutex_unlock(&dev->mode_config.mutex);
        return EINVAL;
    };

    printf("connector %s probed modes :\n", connector->name);

    list_for_each_entry(drmmode, &connector->modes, head)
    {
        printf("%dx%d@%d\n", drmmode->hdisplay, drmmode->vdisplay, drm_mode_vrefresh(drmmode));
    };

    mutex_unlock(&dev->mode_config.mutex);
    return 0;
};

int get_videomodes(videomode_t *mode, int *count)
{
    int err = -1;

//    dbgprintf("mode %x count %d\n", mode, *count);

    if( *count == 0 )
    {
        *count = os_display->supported_modes;
        err = 0;
    }
    else if( mode != NULL )
    {
        struct drm_display_mode  *drmmode;
        int i = 0;

        if( *count > os_display->supported_modes)
            *count = os_display->supported_modes;

        list_for_each_entry(drmmode, &os_display->connector->modes, head)
        {
            if( i < *count)
            {
                mode->width  = drmmode->hdisplay;
                mode->height = drmmode->vdisplay;
                mode->bpp    = 32;
                mode->freq   = drm_mode_vrefresh(drmmode);
                i++;
                mode++;
            }
            else break;
        };
        *count = i;
        err = 0;
    };
    return err;
};

int set_user_mode(videomode_t *mode)
{

//    dbgprintf("width %d height %d vrefresh %d\n",
//               mode->width, mode->height, mode->freq);

    if( (mode->width  != 0)  &&
        (mode->height != 0)  &&
        (mode->freq   != 0 ) &&
        ( (mode->width   != os_display->width)  ||
          (mode->height  != os_display->height) ||
          (mode->freq    != os_display->vrefresh) ) )
    {
        return set_mode(os_display->ddev, os_display->connector, os_display->crtc, mode, true);
    };

    return -1;
};

void i915_dpms(struct drm_device *dev, int mode)
{
    const struct drm_connector_funcs *f = os_display->connector->funcs;

    f->dpms(os_display->connector, mode);
};


int i915_fbinfo(struct drm_i915_fb_info *fb)
{
    u32 ifl;

    ifl = safe_cli();
    {
        struct drm_i915_private *dev_priv = os_display->ddev->dev_private;
        struct intel_crtc *crtc = to_intel_crtc(os_display->crtc);
        struct kos_framebuffer *kfb = os_display->current_lfb;
        struct intel_framebuffer *intel_fb = (struct intel_framebuffer*)kfb->private;
        struct drm_i915_gem_object *obj = intel_fb->obj;

        fb->name   = obj->base.name;
        fb->width  = os_display->width;
        fb->height = os_display->height;
        fb->pitch  = os_display->lfb_pitch;
        fb->tiling = obj->tiling_mode;
        fb->crtc   = crtc->base.base.id;
        fb->pipe   = crtc->pipe;
    }
    safe_sti(ifl);

    return 0;
}


typedef struct
{
    int left;
    int top;
    int right;
    int bottom;
}rect_t;


#define CURRENT_TASK             (0x80003000)

void  FASTCALL GetWindowRect(rect_t *rc)__asm__("GetWindowRect");

int i915_mask_update(struct drm_device *dev, void *data,
            struct drm_file *file)
{
    struct drm_i915_mask *mask = data;
    struct drm_gem_object *obj;
    static unsigned int mask_seqno[256];
    rect_t winrc;
    u32    slot;
    int    ret=0;

    obj = drm_gem_object_lookup(dev, file, mask->handle);
    if (obj == NULL)
        return -ENOENT;

    if (!obj->filp) {
        drm_gem_object_unreference_unlocked(obj);
        return -EINVAL;
    }

    GetWindowRect(&winrc);
    {
//        static warn_count;

        mask->width    = winrc.right - winrc.left + 1;
        mask->height   = winrc.bottom - winrc.top + 1;
        mask->bo_pitch = (mask->width+15) & ~15;

#if 0
        if(warn_count < 1)
        {
            printf("left %d top %d right %d bottom %d\n",
                    winrc.left, winrc.top, winrc.right, winrc.bottom);
            printf("mask pitch %d data %p\n", mask->bo_pitch, mask->bo_map);
            warn_count++;
        };
#endif

     };


    slot = *((u8*)CURRENT_TASK);

    if( mask_seqno[slot] != os_display->mask_seqno)
    {
        u8* src_offset;
        u8* dst_offset;
        u32 ifl;

        ret = i915_mutex_lock_interruptible(dev);
        if (ret)
            goto err1;

        ret = i915_gem_object_set_to_cpu_domain(to_intel_bo(obj), true);
        if(ret != 0 )
        {
            dbgprintf("%s: i915_gem_object_set_to_cpu_domain failed\n", __FUNCTION__);
            goto err2;
        };

//        printf("width %d height %d\n", winrc.right, winrc.bottom);

//        slot = 0x01;

        src_offset = os_display->win_map;
        src_offset+= winrc.top*os_display->width + winrc.left;

        dst_offset = (u8*)mask->bo_map;

        u32 tmp_h = mask->height;

        ifl = safe_cli();
        {
            mask_seqno[slot] = os_display->mask_seqno;

            slot|= (slot<<8)|(slot<<16)|(slot<<24);

            __asm__ __volatile__ (
            "movd       %[slot],   %%xmm6         \n"
            "punpckldq  %%xmm6, %%xmm6            \n"
            "punpcklqdq %%xmm6, %%xmm6            \n"
            :: [slot]  "m" (slot)
            :"xmm6");

            while( tmp_h--)
            {
                int tmp_w = mask->width;

                u8* tmp_src = src_offset;
                u8* tmp_dst = dst_offset;

                src_offset+= os_display->width;
                dst_offset+= mask->bo_pitch;

                while(tmp_w >= 64)
                {
                    __asm__ __volatile__ (
                    "movdqu     (%0),   %%xmm0            \n"
                    "movdqu   16(%0),   %%xmm1            \n"
                    "movdqu   32(%0),   %%xmm2            \n"
                    "movdqu   48(%0),   %%xmm3            \n"
                    "pcmpeqb    %%xmm6, %%xmm0            \n"
                    "pcmpeqb    %%xmm6, %%xmm1            \n"
                    "pcmpeqb    %%xmm6, %%xmm2            \n"
                    "pcmpeqb    %%xmm6, %%xmm3            \n"
                    "movdqa     %%xmm0,   (%%edi)         \n"
                    "movdqa     %%xmm1, 16(%%edi)         \n"
                    "movdqa     %%xmm2, 32(%%edi)         \n"
                    "movdqa     %%xmm3, 48(%%edi)         \n"

                    :: "r" (tmp_src), "D" (tmp_dst)
                    :"xmm0","xmm1","xmm2","xmm3");
                    tmp_w -= 64;
                    tmp_src += 64;
                    tmp_dst += 64;
                }

                if( tmp_w >= 32 )
                {
                    __asm__ __volatile__ (
                    "movdqu     (%0),   %%xmm0            \n"
                    "movdqu   16(%0),   %%xmm1            \n"
                    "pcmpeqb    %%xmm6, %%xmm0            \n"
                    "pcmpeqb    %%xmm6, %%xmm1            \n"
                    "movdqa     %%xmm0,   (%%edi)         \n"
                    "movdqa     %%xmm1, 16(%%edi)         \n"

                    :: "r" (tmp_src), "D" (tmp_dst)
                    :"xmm0","xmm1");
                    tmp_w -= 32;
                    tmp_src += 32;
                    tmp_dst += 32;
                }

                if( tmp_w >= 16 )
                {
                    __asm__ __volatile__ (
                    "movdqu     (%0),   %%xmm0            \n"
                    "pcmpeqb    %%xmm6, %%xmm0            \n"
                    "movdqa     %%xmm0,   (%%edi)         \n"
                    :: "r" (tmp_src), "D" (tmp_dst)
                    :"xmm0");
                    tmp_w -= 16;
                    tmp_src += 16;
                    tmp_dst += 16;
                }

                if( tmp_w >= 8 )
                {
                    __asm__ __volatile__ (
                    "movq       (%0),   %%xmm0            \n"
                    "pcmpeqb    %%xmm6, %%xmm0            \n"
                    "movq       %%xmm0,   (%%edi)         \n"
                    :: "r" (tmp_src), "D" (tmp_dst)
                    :"xmm0");
                    tmp_w -= 8;
                    tmp_src += 8;
                    tmp_dst += 8;
                }
                if( tmp_w >= 4 )
                {
                    __asm__ __volatile__ (
                    "movd       (%0),   %%xmm0            \n"
                    "pcmpeqb    %%xmm6, %%xmm0            \n"
                    "movd       %%xmm0,   (%%edi)         \n"
                    :: "r" (tmp_src), "D" (tmp_dst)
                    :"xmm0");
                    tmp_w -= 4;
                    tmp_src += 4;
                    tmp_dst += 4;
                }
                while(tmp_w--)
                    *tmp_dst++ = (*tmp_src++ == (u8)slot) ? 0xFF:0x00;
            };
        };
        safe_sti(ifl);

        ret = i915_gem_object_set_to_gtt_domain(to_intel_bo(obj), false);
    }

err2:
    mutex_unlock(&dev->struct_mutex);
err1:
    drm_gem_object_unreference(obj);

    return ret;
}

int i915_mask_update_ex(struct drm_device *dev, void *data,
            struct drm_file *file)
{
    struct drm_i915_mask_update *mask = data;
    struct drm_gem_object *obj;
    static unsigned int mask_seqno[256];
    static int warn_count;

    rect_t win;
    u32    winw,winh;
    u32    ml,mt,mr,mb;
    u32    slot;
    int    ret = 0;
    slot = *((u8*)CURRENT_TASK);

    if( mask->forced == 0 && mask_seqno[slot] == os_display->mask_seqno)
        return 0;

    if(mask->forced)
        memset((void*)mask->bo_map,0,mask->width * mask->height);

    GetWindowRect(&win);
    win.right+= 1;
    win.bottom+=  1;

    winw = win.right - win.left;
    winh = win.bottom - win.top;

    if(mask->dx >= winw ||
       mask->dy >= winh)
       return 1;

    ml = win.left + mask->dx;
    mt = win.top  + mask->dy;
    mr = ml + mask->width;
    mb = mt + mask->height;

    if( ml >= win.right || mt >= win.bottom ||
        mr < win.left   || mb < win.top )
        return 1;

    if( mr > win.right )
        mr = win.right;

    if( mb > win.bottom )
        mb = win.bottom;

    mask->width  = mr - ml;
    mask->height = mb - mt;

    if( mask->width == 0 ||
        mask->height== 0 )
        return 1;

    ret = i915_mutex_lock_interruptible(dev);
    if (ret)
        return ret;

    obj = drm_gem_object_lookup(dev, file, mask->handle);
    if (obj == NULL)
    {
        ret = -ENOENT;
        goto unlock;
    }

    if (!obj->filp)
    {
        ret = -ENOENT;
        goto out;
    }

#if 0
    if(warn_count < 100)
    {
        printf("left %d top %d right %d bottom %d\n",
                ml, mt, mr, mb);
        warn_count++;
    };
#endif


#if 1

    {
        u8* src_offset;
        u8* dst_offset;
        u32 ifl;

        i915_gem_object_set_to_cpu_domain(to_intel_bo(obj), true);

        src_offset = os_display->win_map;
        src_offset+= mt*os_display->width + ml;
        dst_offset = (u8*)mask->bo_map;

        u32 tmp_h = mask->height;

        ifl = safe_cli();
        {
            mask_seqno[slot] = os_display->mask_seqno;

            slot|= (slot<<8)|(slot<<16)|(slot<<24);

            __asm__ __volatile__ (
            "movd       %[slot],   %%xmm6         \n"
            "punpckldq  %%xmm6, %%xmm6            \n"
            "punpcklqdq %%xmm6, %%xmm6            \n"
            :: [slot]  "m" (slot)
            :"xmm6");

            while( tmp_h--)
            {
                int tmp_w = mask->width;

                u8* tmp_src = src_offset;
                u8* tmp_dst = dst_offset;

                src_offset+= os_display->width;
                dst_offset+= mask->bo_pitch;

                while(tmp_w >= 64)
                {
                    __asm__ __volatile__ (
                    "movdqu     (%0),   %%xmm0            \n"
                    "movdqu   16(%0),   %%xmm1            \n"
                    "movdqu   32(%0),   %%xmm2            \n"
                    "movdqu   48(%0),   %%xmm3            \n"
                    "pcmpeqb    %%xmm6, %%xmm0            \n"
                    "pcmpeqb    %%xmm6, %%xmm1            \n"
                    "pcmpeqb    %%xmm6, %%xmm2            \n"
                    "pcmpeqb    %%xmm6, %%xmm3            \n"
                    "movdqa     %%xmm0,   (%%edi)         \n"
                    "movdqa     %%xmm1, 16(%%edi)         \n"
                    "movdqa     %%xmm2, 32(%%edi)         \n"
                    "movdqa     %%xmm3, 48(%%edi)         \n"

                    :: "r" (tmp_src), "D" (tmp_dst)
                    :"xmm0","xmm1","xmm2","xmm3");
                    tmp_w -= 64;
                    tmp_src += 64;
                    tmp_dst += 64;
                }

                if( tmp_w >= 32 )
                {
                    __asm__ __volatile__ (
                    "movdqu     (%0),   %%xmm0            \n"
                    "movdqu   16(%0),   %%xmm1            \n"
                    "pcmpeqb    %%xmm6, %%xmm0            \n"
                    "pcmpeqb    %%xmm6, %%xmm1            \n"
                    "movdqa     %%xmm0,   (%%edi)         \n"
                    "movdqa     %%xmm1, 16(%%edi)         \n"

                    :: "r" (tmp_src), "D" (tmp_dst)
                    :"xmm0","xmm1");
                    tmp_w -= 32;
                    tmp_src += 32;
                    tmp_dst += 32;
                }

                if( tmp_w >= 16 )
                {
                    __asm__ __volatile__ (
                    "movdqu     (%0),   %%xmm0            \n"
                    "pcmpeqb    %%xmm6, %%xmm0            \n"
                    "movdqa     %%xmm0,   (%%edi)         \n"
                    :: "r" (tmp_src), "D" (tmp_dst)
                    :"xmm0");
                    tmp_w -= 16;
                    tmp_src += 16;
                    tmp_dst += 16;
                }

                if( tmp_w >= 8 )
                {
                    __asm__ __volatile__ (
                    "movq       (%0),   %%xmm0            \n"
                    "pcmpeqb    %%xmm6, %%xmm0            \n"
                    "movq       %%xmm0,   (%%edi)         \n"
                    :: "r" (tmp_src), "D" (tmp_dst)
                    :"xmm0");
                    tmp_w -= 8;
                    tmp_src += 8;
                    tmp_dst += 8;
                }
                if( tmp_w >= 4 )
                {
                    __asm__ __volatile__ (
                    "movd       (%0),   %%xmm0            \n"
                    "pcmpeqb    %%xmm6, %%xmm0            \n"
                    "movd       %%xmm0,   (%%edi)         \n"
                    :: "r" (tmp_src), "D" (tmp_dst)
                    :"xmm0");
                    tmp_w -= 4;
                    tmp_src += 4;
                    tmp_dst += 4;
                }
                while(tmp_w--)
                    *tmp_dst++ = (*tmp_src++ == (u8)slot) ? 0xFF:0x00;
            };
        };
        safe_sti(ifl);

        i915_gem_object_set_to_gtt_domain(to_intel_bo(obj), false);
    }
#endif

out:
    drm_gem_object_unreference(obj);

unlock:
    mutex_unlock(&dev->struct_mutex);

    return ret;
}




#define NSEC_PER_SEC    1000000000L

void getrawmonotonic(struct timespec *ts)
{
    u32 tmp = GetTimerTicks();

    ts->tv_sec  = tmp/100;
    ts->tv_nsec = (tmp - ts->tv_sec*100)*10000000;
}

void prepare_to_wait(wait_queue_head_t *q, wait_queue_t *wait, int state)
{
    unsigned long flags;

//    wait->flags &= ~WQ_FLAG_EXCLUSIVE;
    spin_lock_irqsave(&q->lock, flags);
    if (list_empty(&wait->task_list))
            __add_wait_queue(q, wait);
    spin_unlock_irqrestore(&q->lock, flags);
}

/**
 * finish_wait - clean up after waiting in a queue
 * @q: waitqueue waited on
 * @wait: wait descriptor
 *
 * Sets current thread back to running state and removes
 * the wait descriptor from the given waitqueue if still
 * queued.
 */
void finish_wait(wait_queue_head_t *q, wait_queue_t *wait)
{
    unsigned long flags;

//    __set_current_state(TASK_RUNNING);
    /*
     * We can check for list emptiness outside the lock
     * IFF:
     *  - we use the "careful" check that verifies both
     *    the next and prev pointers, so that there cannot
     *    be any half-pending updates in progress on other
     *    CPU's that we haven't seen yet (and that might
     *    still change the stack area.
     * and
     *  - all other users take the lock (ie we can only
     *    have _one_ other CPU that looks at or modifies
     *    the list).
     */
    if (!list_empty_careful(&wait->task_list)) {
            spin_lock_irqsave(&q->lock, flags);
            list_del_init(&wait->task_list);
            spin_unlock_irqrestore(&q->lock, flags);
    }

    DestroyEvent(wait->evnt);
}

int autoremove_wake_function(wait_queue_t *wait, unsigned mode, int sync, void *key)
{
    list_del_init(&wait->task_list);
    return 1;
}

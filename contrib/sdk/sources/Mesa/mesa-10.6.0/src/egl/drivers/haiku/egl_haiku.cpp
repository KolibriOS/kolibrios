/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2014 Adrián Arroyo Calle <adrian.arroyocalle@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <errno.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>

#include "loader.h"
#include "eglconfig.h"
#include "eglcontext.h"
#include "egldisplay.h"
#include "egldriver.h"
#include "eglcurrent.h"
#include "egllog.h"
#include "eglsurface.h"
#include "eglimage.h"
#include "egltypedefs.h"

#include <InterfaceKit.h>
#include <OpenGLKit.h>


#define CALLOC_STRUCT(T)   (struct T *) calloc(1, sizeof(struct T))


_EGL_DRIVER_STANDARD_TYPECASTS(haiku_egl)


struct haiku_egl_driver
{
	_EGLDriver base;

	void *handle;
	_EGLProc (*get_proc_address)(const char *procname);
	void (*glFlush)(void);
};

struct haiku_egl_config
{
	_EGLConfig         base;
};

struct haiku_egl_context
{
	_EGLContext	ctx;
};

struct haiku_egl_surface
{
	_EGLSurface surf;
	BGLView* gl;
};


/*
static void
swrastCreateDrawable(struct dri2_egl_display * dri2_dpy,
	struct dri2_egl_surface * dri2_surf, int depth)
{

}


static void
swrastDestroyDrawable(struct dri2_egl_display * dri2_dpy,
	struct dri2_egl_surface * dri2_surf)
{

}


static void
swrastGetDrawableInfo(__DRIdrawable * draw, int *x, int *y,
	int *w, int *h, void *loaderPrivate)
{

}


static void
swrastPutImage(__DRIdrawable * draw, int op, int x, int y,
	int w, int h, char *data, void *loaderPrivate)
{

}


static void
swrastGetImage(__DRIdrawable * read, int x, int y,
	int w, int h, char *data, void *loaderPrivate)
{

}
*/


static void
haiku_log(EGLint level, const char *msg)
{
	switch (level) {
		case _EGL_DEBUG:
			fprintf(stderr,"%s", msg);
			break;
		case _EGL_INFO:
			fprintf(stderr,"%s", msg);
			break;
		case _EGL_WARNING:
			fprintf(stderr,"%s", msg);
			break;
		case _EGL_FATAL:
			fprintf(stderr,"%s", msg);
			break;
		default:
			break;
	}
}


/**
 * Called via eglCreateWindowSurface(), drv->API.CreateWindowSurface().
 */
static _EGLSurface *
haiku_create_surface(_EGLDriver *drv, _EGLDisplay *disp, EGLint type,
	_EGLConfig *conf, void *native_surface, const EGLint *attrib_list)
{
	return NULL;
}


/**
 * Called via eglCreateWindowSurface(), drv->API.CreateWindowSurface().
 */
static _EGLSurface *
haiku_create_window_surface(_EGLDriver *drv, _EGLDisplay *disp,
	_EGLConfig *conf, void *native_window, const EGLint *attrib_list)
{
	struct haiku_egl_surface* surface;
	surface = (struct haiku_egl_surface*)calloc(1,sizeof (*surface));

	_eglInitSurface(&surface->surf, disp, EGL_WINDOW_BIT, conf, attrib_list);
	(&surface->surf)->SwapInterval = 1;

	_eglLog(_EGL_DEBUG, "Creating window");
	BWindow* win = (BWindow*)native_window;

	_eglLog(_EGL_DEBUG, "Creating GL view");
	surface->gl = new BGLView(win->Bounds(), "OpenGL", B_FOLLOW_ALL_SIDES, 0,
		BGL_RGB | BGL_DOUBLE | BGL_ALPHA);

	_eglLog(_EGL_DEBUG, "Adding GL");
	win->AddChild(surface->gl);

	_eglLog(_EGL_DEBUG, "Showing window");
	win->Show();
	return &surface->surf;
}


static _EGLSurface *
haiku_create_pixmap_surface(_EGLDriver *drv, _EGLDisplay *disp,
	_EGLConfig *conf, void *native_pixmap, const EGLint *attrib_list)
{
	return NULL;
}


static _EGLSurface *
haiku_create_pbuffer_surface(_EGLDriver *drv, _EGLDisplay *disp,
	_EGLConfig *conf, const EGLint *attrib_list)
{
	return NULL;
}


static EGLBoolean
haiku_destroy_surface(_EGLDriver *drv, _EGLDisplay *disp, _EGLSurface *surf)
{
	return EGL_TRUE;
}


static EGLBoolean
haiku_add_configs_for_visuals(_EGLDisplay *dpy)
{
	printf("Adding configs\n");

	struct haiku_egl_config* conf;
	conf = CALLOC_STRUCT(haiku_egl_config);

	_eglInitConfig(&conf->base, dpy, 1);
	_eglLog(_EGL_DEBUG,"Config inited\n");
	_eglSetConfigKey(&conf->base, EGL_RED_SIZE, 8);
	_eglSetConfigKey(&conf->base, EGL_BLUE_SIZE, 8);
	_eglSetConfigKey(&conf->base, EGL_GREEN_SIZE, 8);
	_eglSetConfigKey(&conf->base, EGL_LUMINANCE_SIZE, 0);
	_eglSetConfigKey(&conf->base, EGL_ALPHA_SIZE, 8);
	_eglSetConfigKey(&conf->base, EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER);
	EGLint r = (_eglGetConfigKey(&conf->base, EGL_RED_SIZE) 
		+ _eglGetConfigKey(&conf->base, EGL_GREEN_SIZE)
		+ _eglGetConfigKey(&conf->base, EGL_BLUE_SIZE)
		+ _eglGetConfigKey(&conf->base, EGL_ALPHA_SIZE));
	_eglSetConfigKey(&conf->base, EGL_BUFFER_SIZE, r);
	_eglSetConfigKey(&conf->base, EGL_CONFIG_CAVEAT, EGL_NONE);
	_eglSetConfigKey(&conf->base, EGL_CONFIG_ID, 1);
	_eglSetConfigKey(&conf->base, EGL_BIND_TO_TEXTURE_RGB, EGL_FALSE);
	_eglSetConfigKey(&conf->base, EGL_BIND_TO_TEXTURE_RGBA, EGL_FALSE);
	_eglSetConfigKey(&conf->base, EGL_STENCIL_SIZE, 0);
	_eglSetConfigKey(&conf->base, EGL_TRANSPARENT_TYPE, EGL_NONE);
	_eglSetConfigKey(&conf->base, EGL_NATIVE_RENDERABLE, EGL_TRUE); // Let's say yes
	_eglSetConfigKey(&conf->base, EGL_NATIVE_VISUAL_ID, 0); // No visual
	_eglSetConfigKey(&conf->base, EGL_NATIVE_VISUAL_TYPE, EGL_NONE); // No visual
	_eglSetConfigKey(&conf->base, EGL_RENDERABLE_TYPE, 0x8);
	_eglSetConfigKey(&conf->base, EGL_SAMPLE_BUFFERS, 0); // TODO: How to get the right value ?
	_eglSetConfigKey(&conf->base, EGL_SAMPLES, _eglGetConfigKey(&conf->base, EGL_SAMPLE_BUFFERS) == 0 ? 0 : 0);
	_eglSetConfigKey(&conf->base, EGL_DEPTH_SIZE, 24); // TODO: How to get the right value ?
	_eglSetConfigKey(&conf->base, EGL_LEVEL, 0);
	_eglSetConfigKey(&conf->base, EGL_MAX_PBUFFER_WIDTH, 0); // TODO: How to get the right value ?
	_eglSetConfigKey(&conf->base, EGL_MAX_PBUFFER_HEIGHT, 0); // TODO: How to get the right value ?
	_eglSetConfigKey(&conf->base, EGL_MAX_PBUFFER_PIXELS, 0); // TODO: How to get the right value ?
	_eglSetConfigKey(&conf->base, EGL_SURFACE_TYPE, EGL_WINDOW_BIT /*| EGL_PIXMAP_BIT | EGL_PBUFFER_BIT*/);

	printf("Config configuated\n");
	if (!_eglValidateConfig(&conf->base, EGL_FALSE)) {
		_eglLog(_EGL_DEBUG, "Haiku failed to validate config");
		return EGL_FALSE;
	}
	printf("Validated config\n");
   
	_eglLinkConfig(&conf->base);
	if (!_eglGetArraySize(dpy->Configs)) {
		_eglLog(_EGL_WARNING, "Haiku: failed to create any config");
		return EGL_FALSE;
	}
	printf("Config successful!\n");
   
	return EGL_TRUE;
}

extern "C"
EGLBoolean
init_haiku(_EGLDriver *drv, _EGLDisplay *dpy)
{
	_eglLog(_EGL_DEBUG,"\nInitializing Haiku EGL\n");
	//_EGLDisplay* egl_dpy;

	printf("Initializing Haiku EGL\n");
	_eglSetLogProc(haiku_log);

	loader_set_logger(_eglLog);

	/*egl_dpy = (_EGLDisplay*) calloc(1, sizeof(_EGLDisplay));
	if (!egl_dpy)
		return _eglError(EGL_BAD_ALLOC, "eglInitialize");

	dpy->DriverData=(void*) egl_dpy;
	if (!dpy->PlatformDisplay) {
		// OPEN DEVICE 
		//dri2_dpy->bwindow = (void*)haiku_create_window();
		//dri2_dpy->own_device = true;
	} else {
		//dri2_dpy->bwindow = (BWindow*)dpy->PlatformDisplay;
	}*/
	
	//dri2_dpy->driver_name = strdup("swrast");
	//if (!dri2_load_driver_swrast(dpy))
	//   goto cleanup_conn;

	/*dri2_dpy->swrast_loader_extension.base.name = __DRI_SWRAST_LOADER;
	dri2_dpy->swrast_loader_extension.base.version = __DRI_SWRAST_LOADER_VERSION;
	dri2_dpy->swrast_loader_extension.getDrawableInfo = swrastGetDrawableInfo;
	dri2_dpy->swrast_loader_extension.putImage = swrastPutImage;
	dri2_dpy->swrast_loader_extension.getImage = swrastGetImage;

	dri2_dpy->extensions[0] = &dri2_dpy->swrast_loader_extension.base;
	dri2_dpy->extensions[1] = NULL;
	dri2_dpy->extensions[2] = NULL;*/

	/*if (dri2_dpy->bwindow) {
		if (!dri2_haiku_add_configs_for_visuals(dri2_dpy, dpy))
			goto cleanup_configs;
	}*/
	_eglLog(_EGL_DEBUG,"Add configs");
    haiku_add_configs_for_visuals(dpy);

	dpy->VersionMajor=1;
	dpy->VersionMinor=4;
   
   //dpy->Extensions.KHR_create_context = true;

	//dri2_dpy->vtbl = &dri2_haiku_display_vtbl;
	_eglLog(_EGL_DEBUG, "Initialization finished");

	return EGL_TRUE;
}


extern "C"
EGLBoolean
haiku_terminate(_EGLDriver* drv,_EGLDisplay* dpy)
{
	return EGL_TRUE;
}


extern "C"
_EGLContext*
haiku_create_context(_EGLDriver *drv, _EGLDisplay *disp, _EGLConfig *conf,
	_EGLContext *share_list, const EGLint *attrib_list)
{
	_eglLog(_EGL_DEBUG,"Creating context");
	struct haiku_egl_context* context;
	context=(struct haiku_egl_context*)calloc(1,sizeof (*context));
	if(!_eglInitContext(&context->ctx, disp, conf, attrib_list))
		printf("ERROR creating context");
	_eglLog(_EGL_DEBUG, "Context created");
	return &context->ctx;
}


extern "C"
EGLBoolean
haiku_destroy_context(_EGLDriver* drv, _EGLDisplay *disp, _EGLContext* ctx)
{
	ctx=NULL;
	return EGL_TRUE;
}


extern "C"
EGLBoolean
haiku_make_current(_EGLDriver* drv, _EGLDisplay* dpy, _EGLSurface *dsurf,
		  _EGLSurface *rsurf, _EGLContext *ctx)
{
	struct haiku_egl_context* cont=haiku_egl_context(ctx);
	struct haiku_egl_surface* surf=haiku_egl_surface(dsurf);
	_EGLContext *old_ctx;
    _EGLSurface *old_dsurf, *old_rsurf;
	_eglBindContext(ctx, dsurf, rsurf, &old_ctx, &old_dsurf, &old_rsurf);
	//cont->ctx.DrawSurface=&surf->surf;
	surf->gl->LockGL();
	return EGL_TRUE;
}


extern "C"
EGLBoolean
haiku_swap_buffers(_EGLDriver *drv, _EGLDisplay *dpy, _EGLSurface *surf)
{
	struct haiku_egl_surface* surface=haiku_egl_surface(surf);
	surface->gl->SwapBuffers();
	//gl->Render();
	return EGL_TRUE;
}


extern "C"
void
haiku_unload(_EGLDriver* drv)
{
	
}


/**
 * This is the main entrypoint into the driver, called by libEGL.
 * Create a new _EGLDriver object and init its dispatch table.
 */
extern "C"
_EGLDriver*
_eglBuiltInDriverHaiku(const char *args)
{
	_eglLog(_EGL_DEBUG,"Driver loaded");
	struct haiku_egl_driver* driver;
	driver=(struct haiku_egl_driver*)calloc(1,sizeof(*driver));
	_eglInitDriverFallbacks(&driver->base);
	driver->base.API.Initialize = init_haiku;
	driver->base.API.Terminate = haiku_terminate;
	driver->base.API.CreateContext = haiku_create_context;
	driver->base.API.DestroyContext = haiku_destroy_context;
	driver->base.API.MakeCurrent = haiku_make_current;
	driver->base.API.CreateWindowSurface = haiku_create_window_surface;
	driver->base.API.CreatePixmapSurface = haiku_create_pixmap_surface;
	driver->base.API.CreatePbufferSurface = haiku_create_pbuffer_surface;
	driver->base.API.DestroySurface = haiku_destroy_surface;
	/*
	driver->API.GetProcAddress = dri2_get_proc_address;
	driver->API.WaitClient = dri2_wait_client;
	driver->API.WaitNative = dri2_wait_native;
	driver->API.BindTexImage = dri2_bind_tex_image;
	driver->API.ReleaseTexImage = dri2_release_tex_image;
	driver->API.SwapInterval = dri2_swap_interval;
	*/

	driver->base.API.SwapBuffers = haiku_swap_buffers;
	/*
	driver->API.SwapBuffersWithDamageEXT = dri2_swap_buffers_with_damage;
	driver->API.SwapBuffersRegionNOK = dri2_swap_buffers_region;
	driver->API.PostSubBufferNV = dri2_post_sub_buffer;
	driver->API.CopyBuffers = dri2_copy_buffers,
	driver->API.QueryBufferAge = dri2_query_buffer_age;
	driver->API.CreateImageKHR = dri2_create_image;
	driver->API.DestroyImageKHR = dri2_destroy_image_khr;
	driver->API.CreateWaylandBufferFromImageWL = dri2_create_wayland_buffer_from_image;
	driver->API.GetSyncValuesCHROMIUM = dri2_get_sync_values_chromium;
	*/

	driver->base.Name = "Haiku";
	driver->base.Unload = haiku_unload;

	_eglLog(_EGL_DEBUG, "API Calls defined");
	
	return &driver->base;
}

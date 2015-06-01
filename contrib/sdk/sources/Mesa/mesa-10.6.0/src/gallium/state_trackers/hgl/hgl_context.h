/*
 * Copyright 2009-2014, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Alexander von Gluck IV, kallisti5@unixzen.com
 */
#ifndef HGL_CONTEXT_H
#define HGL_CONTEXT_H


#include "state_tracker/st_api.h"
#include "state_tracker/st_manager.h"
#include "pipe/p_compiler.h"
#include "pipe/p_screen.h"
#include "postprocess/filters.h"
#include "os/os_thread.h"

#include "bitmap_wrapper.h"


#ifdef __cplusplus
extern "C" {
#endif


#define CONTEXT_MAX 32

typedef int64 context_id;


struct hgl_buffer
{
	struct st_framebuffer_iface *stfbi;
	struct st_visual* visual;

	unsigned width;
	unsigned height;
	unsigned mask;

	struct pipe_screen* screen;
	enum pipe_texture_target target;
	struct pipe_resource* textures[ST_ATTACHMENT_COUNT];

	void *map;

	//struct hgl_buffer *next;  /**< next in linked list */
};


struct hgl_context
{
	struct st_api* api;
		// State Tracker API
	struct st_manager* manager;
		// State Tracker Manager
	struct st_context_iface* st;
		// State Tracker Interface Object
	struct st_visual* stVisual;
		// State Tracker Visual

	struct pipe_screen* screen;

	//struct pipe_resource* textures[ST_ATTACHMENT_COUNT];

	// Post processing
	struct pp_queue_t* postProcess;
	unsigned int postProcessEnable[PP_FILTERS];

	// Desired viewport size
	unsigned width;
	unsigned height;

	Bitmap* bitmap;
	color_space colorSpace;

	pipe_mutex fbMutex;

	struct hgl_buffer* draw;
	struct hgl_buffer* read;
};


// hgl state_tracker api
struct st_api* hgl_create_st_api(void);

// hgl state_tracker framebuffer
struct hgl_buffer* hgl_create_st_framebuffer(struct hgl_context* context);

// hgl state_tracker manager
struct st_manager* hgl_create_st_manager(struct hgl_context* screen);
void hgl_destroy_st_manager(struct st_manager *manager);

// hgl state_tracker visual
struct st_visual* hgl_create_st_visual(ulong options);
void hgl_destroy_st_visual(struct st_visual* visual);


#ifdef __cplusplus
}
#endif

#endif /* HGL_CONTEXT_H */

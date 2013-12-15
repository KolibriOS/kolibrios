/*
 * Copyright 2005 Richard Wilson <info@tinct.net>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 * Content for image/mng, image/png, and image/jng (implementation).
 */

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <libmng.h>
#include "content/content_protected.h"
#include "desktop/options.h"
#include "desktop/plotters.h"
#include "image/bitmap.h"
#include "image/mng.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/schedule.h"
#include "utils/utils.h"

/* This implementation does not currently support dynamic MNGs or any
 * form of colour/gamma correction,
 */

typedef struct nsmng_content
{
	struct content base;
	
	bool opaque_test_pending;
	bool read_start;
	bool read_resume;
	int read_size;
	bool waiting;
	bool displayed;
	void *handle;

	struct bitmap *bitmap;	/**< Created NetSurf bitmap */
} nsmng_content;


#ifndef MNG_INTERNAL_MEMMNGMT

/**
 * Memory allocation callback for libmng.
 */

static mng_ptr nsmng_alloc(mng_size_t n)
{
	return calloc(1, n);
}


/**
 * Memory free callback for libmng.
 */

static void nsmng_free(mng_ptr p, mng_size_t n)
{
	free(p);
}

#endif

/**
 * Broadcasts an error message and returns false
 *
 * \param c the content to broadcast for
 * \return Appropriate error
 */
static nserror nsmng_broadcast_error(nsmng_content *c, mng_retcode code)
{
	union content_msg_data msg_data;
	char error[100];

	assert(c != NULL);

	if (code == MNG_OUTOFMEMORY) {
		msg_data.error = messages_get("NoMemory");
		content_broadcast(&c->base, CONTENT_MSG_ERROR, msg_data);
		return NSERROR_NOMEM;
	}

	snprintf(error, sizeof error, messages_get("MNGError"), code);
	msg_data.error = error;
	content_broadcast(&c->base, CONTENT_MSG_ERROR, msg_data);
	return NSERROR_MNG_ERROR;
}

/* CALLBACKS REQUIRED FOR libmng READING */

static mng_bool nsmng_openstream(mng_handle mng)
{
	assert(mng != NULL);
	return MNG_TRUE;
}

static mng_bool nsmng_readdata(mng_handle mng, mng_ptr buffer, mng_uint32 size, 
		mng_uint32 *bytesread)
{
	nsmng_content *c;
	const char *data;
	unsigned long data_size;

	assert(mng != NULL);
	assert(buffer != NULL);
	assert(bytesread != NULL);

	/*	Get our content back
	*/
	c = (nsmng_content *) mng_get_userdata(mng);
	assert(c != NULL);

	/*	Copy any data we have (maximum of 'size')
	*/
	data = content__get_source_data(&c->base, &data_size);

	*bytesread = ((data_size - c->read_size) < size) ?
			(data_size - c->read_size) : size;

	if ((*bytesread) > 0) {
		memcpy(buffer, data + c->read_size, *bytesread);
		c->read_size += *bytesread;
	}

	/*	Return success
	*/
	return MNG_TRUE;
}

static mng_bool nsmng_closestream(mng_handle mng)
{
	assert(mng != NULL);
	return MNG_TRUE;
}

static mng_bool nsmng_processheader(mng_handle mng, mng_uint32 width, 
		mng_uint32 height)
{
	nsmng_content *c;
	union content_msg_data msg_data;
	uint8_t *buffer;

	assert(mng != NULL);

	/*	This function is called when the header has been read and we 
	 	know the dimensions of the canvas.
	*/
	c = (nsmng_content *) mng_get_userdata(mng);
	assert(c != NULL);

	c->bitmap = bitmap_create(width, height, BITMAP_NEW);
	if (c->bitmap == NULL) {
		msg_data.error = messages_get("NoMemory");
		content_broadcast(&c->base, CONTENT_MSG_ERROR, msg_data);
		LOG(("Insufficient memory to create canvas."));
		return MNG_FALSE;
	}

	/* Get the buffer to ensure that it is allocated and the calls in
	 * nsmng_getcanvasline() succeed. */
	buffer = bitmap_get_buffer(c->bitmap);
	if (buffer == NULL) {
		msg_data.error = messages_get("NoMemory");
		content_broadcast(&c->base, CONTENT_MSG_ERROR, msg_data);
		LOG(("Insufficient memory to create canvas."));
		return MNG_FALSE;
	}

	/*	Initialise the content size
	*/
	c->base.width = width;
	c->base.height = height;

	/*	Set the canvas style
	*/
	if (mng_set_canvasstyle(mng, MNG_CANVAS_RGBA8) != MNG_NOERROR) {
		LOG(("Error setting canvas style."));
	}

	/*	Return success
	*/
	return MNG_TRUE;
}


/*	END OF CALLBACKS REQUIRED FOR READING
*/


static bool nsmng_process_data(struct content *c, const char *data, unsigned int size)
{
	nsmng_content *mng = (nsmng_content *) c;
	mng_retcode status;

	assert(c != NULL);
	assert(data != NULL);

	/*	We only need to do any processing if we're starting/resuming reading.
	*/
	if ((!mng->read_resume) && (!mng->read_start))
		return true;

	/*	Try to start processing, or process some more data
	*/
	if (mng->read_start) {
		status = mng_read(mng->handle);
		mng->read_start = false;
	} else {
		status = mng_read_resume(mng->handle);
	}
	mng->read_resume = (status == MNG_NEEDMOREDATA);
	if ((status != MNG_NOERROR) && (status != MNG_NEEDMOREDATA)) {
		LOG(("Failed to start/continue reading (%i).", status));
		return nsmng_broadcast_error(mng, status) == NSERROR_OK;
	}

	/*	Continue onwards
	*/
	return true;
}

/*	START OF CALLBACKS REQUIRED FOR DISPLAYING
*/


static mng_ptr nsmng_getcanvasline(mng_handle mng, mng_uint32 line)
{
	nsmng_content *c;

	assert(mng != NULL);

	/*	Get our content back
	*/
	c = (nsmng_content *) mng_get_userdata(mng);
	assert(c != NULL);

	/*	Calculate the address
	*/
	return bitmap_get_buffer(c->bitmap) +
			bitmap_get_rowstride(c->bitmap) * line;
}

static mng_bool nsmng_refresh(mng_handle mng, mng_uint32 x, mng_uint32 y, 
		mng_uint32 w, mng_uint32 h)
{
	union content_msg_data data;
	nsmng_content *c;

	assert(mng != NULL);

	/*	Get our content back
	*/
	c = (nsmng_content *) mng_get_userdata(mng);
	assert(c != NULL);

	/*	Set the minimum redraw area
	*/
	data.redraw.x = x;
	data.redraw.y = y;
	data.redraw.width = w;
	data.redraw.height = h;

	/*	Set the redraw area to the whole canvas to ensure that if 
	 	we can redraw something to trigger animation later then we do
	*/
/*	data.redraw.x = 0;
	data.redraw.y = 0;
	data.redraw.width = c->width;
	data.redraw.height = c->height;
*/
	/*	Always redraw everything
	*/
	data.redraw.full_redraw = true;

	/*	Set the object characteristics
	*/
	data.redraw.object = &c->base;
	data.redraw.object_x = 0;
	data.redraw.object_y = 0;
	data.redraw.object_width = c->base.width;
	data.redraw.object_height = c->base.height;

	/* Only attempt to force the redraw if we've been requested to
	 * display the image in the first place (i.e. nsmng_redraw has
	 * been called). This avoids the situation of forcibly redrawing
	 * an image that shouldn't be shown (e.g. if the image is a fallback
	 * for an object that can't be rendered)
	 */
	if (c->displayed)
		content_broadcast(&c->base, CONTENT_MSG_REDRAW, data);

	return MNG_TRUE;
}

/**
 * Animates to the next frame
 */
static void nsmng_animate(void *p)
{
 	nsmng_content *c;

 	assert(p != NULL);

 	c = (nsmng_content *) p;

 	/*	If we used the last animation we advance, if not we try again later
 	*/
 	if (c->base.user_list->next == NULL) {
 		c->waiting = true;
 	} else {
 		c->waiting = false;
 		mng_display_resume(c->handle);
		c->opaque_test_pending = true;
		if (c->bitmap)
			bitmap_modified(c->bitmap);
 	}
}

static mng_bool nsmng_settimer(mng_handle mng, mng_uint32 msecs)
{
	nsmng_content *c;

	assert(mng != NULL);

	/*	Get our content back
	*/
	c = (nsmng_content *) mng_get_userdata(mng);
	assert(c != NULL);

	/*	Perform the scheduling
	*/
	schedule(msecs / 10, nsmng_animate, c);
	return MNG_TRUE;
}

/**
 * Get the wall-clock time in milliseconds since some fixed time.
 */

static mng_uint32 nsmng_gettickcount(mng_handle mng)
{
	static bool start = true;
	static time_t t0;
	struct timeval tv;
#if defined(__SVR4) && defined(__sun) || defined(__NetBSD__) || \
	defined(__APPLE__)
	/* Solaris, NetBSD, and OS X don't have this structure, and ignore the 
	 * second parameter to gettimeofday()
	 */
	int tz;
#else
	struct timezone tz;
#endif
	assert(mng != NULL);

	gettimeofday(&tv, &tz);
	if (start) {
		t0 = tv.tv_sec;
		start = false;
	}

	return (tv.tv_sec - t0) * 1000 + tv.tv_usec / 1000;
}

/*	END OF CALLBACKS REQUIRED FOR DISPLAYING
*/

static mng_bool nsmng_errorproc(mng_handle mng, mng_int32 code,
		mng_int8 severity, mng_chunkid chunktype, mng_uint32 chunkseq,
		mng_int32 extra1, mng_int32 extra2, mng_pchar text)
{
	nsmng_content *c;
	char chunk[5];

	assert(mng != NULL);

	c = (nsmng_content *) mng_get_userdata(mng);
	assert(c != NULL);

	chunk[0] = (char)((chunktype >> 24) & 0xFF);
	chunk[1] = (char)((chunktype >> 16) & 0xFF);
	chunk[2] = (char)((chunktype >>  8) & 0xFF);
	chunk[3] = (char)((chunktype      ) & 0xFF);
	chunk[4] = '\0';

	LOG(("error playing '%s' chunk %s (%d):", 
			content_get_url(&c->base), chunk, chunkseq));
	LOG(("code %d severity %d extra1 %d extra2 %d text:'%s'", code,
					severity, extra1, extra2, text));

	return (0);
}

static nserror nsmng_create_mng_data(nsmng_content *c)
{
	mng_retcode code;
	union content_msg_data msg_data;

	assert(c != NULL);

	/*	Initialise the library
	*/
#ifdef MNG_INTERNAL_MEMMNGMT
	c->handle = mng_initialize(c, MNG_NULL, MNG_NULL, MNG_NULL);
#else
	c->handle = mng_initialize(c, nsmng_alloc, nsmng_free, MNG_NULL);
#endif
	if (c->handle == MNG_NULL) {
		LOG(("Unable to initialise MNG library."));
		msg_data.error = messages_get("NoMemory");
		content_broadcast(&c->base, CONTENT_MSG_ERROR, msg_data);
		return NSERROR_NOMEM;
	}

	/*	We need to decode in suspension mode
	*/
	code = mng_set_suspensionmode(c->handle, MNG_TRUE);
	if (code) {
		LOG(("Unable to set suspension mode."));
		return nsmng_broadcast_error(c, code);
	}

	/*	We need to register our callbacks
	*/
	code = mng_setcb_openstream(c->handle, nsmng_openstream);
	if (code) {
		LOG(("Unable to set openstream callback."));
		return nsmng_broadcast_error(c, code);
	}
	code = mng_setcb_readdata(c->handle, nsmng_readdata);
	if (code) {
		LOG(("Unable to set readdata callback."));
		return nsmng_broadcast_error(c, code);
	}
	code = mng_setcb_closestream(c->handle, nsmng_closestream);
	if (code) {
		LOG(("Unable to set closestream callback."));
		return nsmng_broadcast_error(c, code);
	}
	code = mng_setcb_processheader(c->handle, nsmng_processheader);
	if (code) {
		LOG(("Unable to set processheader callback."));
		return nsmng_broadcast_error(c, code);
	}

	/*	Register our callbacks for displaying
	*/
	code = mng_setcb_getcanvasline(c->handle, nsmng_getcanvasline);
	if (code) {
		LOG(("Unable to set getcanvasline callback."));
		return nsmng_broadcast_error(c, code);
	}
	code = mng_setcb_refresh(c->handle, nsmng_refresh);
	if (code) {
		LOG(("Unable to set refresh callback."));
		return nsmng_broadcast_error(c, code);
	}
	code = mng_setcb_gettickcount(c->handle, nsmng_gettickcount);
	if (code) {
		LOG(("Unable to set gettickcount callback."));
		return nsmng_broadcast_error(c, code);
	}
	code = mng_setcb_settimer(c->handle, nsmng_settimer);
	if (code) {
		LOG(("Unable to set settimer callback."));
		return nsmng_broadcast_error(c, code);
	}

	/* register error handling function */
	code = mng_setcb_errorproc(c->handle, nsmng_errorproc);
	if (code) {
		LOG(("Unable to set errorproc"));
		return nsmng_broadcast_error(c, code);
	}

	/*	Initialise the reading
	*/
	c->read_start = true;
	c->read_resume = false;
	c->read_size = 0;
	c->waiting = false;

	c->displayed = false;

	return NSERROR_OK;
}

static nserror nsmng_create(const content_handler *handler, 
		lwc_string *imime_type, const struct http_parameter *params,
		llcache_handle *llcache, const char *fallback_charset,
		bool quirks, struct content **c)
{
	nsmng_content *mng;
	nserror error;

	mng = calloc(1, sizeof(nsmng_content));
	if (mng == NULL)
		return NSERROR_NOMEM;

	error = content__init(&mng->base, handler, imime_type, params,
			llcache, fallback_charset, quirks);
	if (error != NSERROR_OK) {
		free(mng);
		return error;
	}

	error = nsmng_create_mng_data(mng);
	if (error != NSERROR_OK) {
		free(mng);
		return error;
	}

	*c = (struct content *) mng;

	return NSERROR_OK;
}



static bool nsmng_convert(struct content *c)
{
	nsmng_content *mng = (nsmng_content *) c;
	mng_retcode status;
	unsigned long size;
	char *title;

	assert(c != NULL);

	content__get_source_data(c, &size);

	/* by this point, the png should have been parsed
	 * and the bitmap created, so ensure that's the case
	 */
	if (mng->bitmap == NULL) {
		return nsmng_broadcast_error(mng, -1) == NSERROR_OK;
	}


	/* set title text */
	title = messages_get_buff("MNGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	if (title != NULL) {
		content__set_title(c, title);
		free(title);
	}

	c->size += c->width * c->height * 4;
	content_set_ready(c);
	content_set_done(c);
	/* Done: update status bar */
	content_set_status(c, "");

	/* jmb: I'm really not sure that this should be here.
	 * The *_convert functions are for converting a content into a
	 * displayable format. They should not, however, do anything which
	 * could cause the content to be displayed; the content may have
	 * hidden visibility or be a fallback for an object; this
	 * information is not available here (nor is there any need for it
	 * to be).
	 * The specific issue here is that mng_display calls the display
	 * callbacks, which include nsmng_refresh. nsmng_refresh forces
	 * a content to be redrawn regardless of whether it should be
	 * displayed or not.
	 */
	/*	Start displaying
	*/
	status = mng_display(mng->handle);
	if ((status != MNG_NOERROR) && (status != MNG_NEEDTIMERWAIT)) {
		LOG(("Unable to start display (%i)", status));
		return nsmng_broadcast_error(mng, status) == NSERROR_OK;
	}
	bitmap_modified(mng->bitmap);

	/* Optimise the plotting of MNG */
	mng->opaque_test_pending = false;

	return true;
}

static bool nsjpng_convert(struct content *c)
{
	nsmng_content *mng = (nsmng_content *) c;
	mng_retcode status;
	unsigned long size;
	char *title;
	mng_handle handle;

	assert(c != NULL);

	content__get_source_data(c, &size);

	/* by this point, the png should have been parsed
	 * and the bitmap created, so ensure that's the case
	 */
	if (mng->bitmap == NULL) {
		return nsmng_broadcast_error(mng, -1) == NSERROR_OK;
	}

	/* set title text */
	title = messages_get_buff("PNGTitle",
			nsurl_access_leaf(llcache_handle_get_url(c->llcache)),
			c->width, c->height);
	if (title != NULL) {
		content__set_title(c, title);
		free(title);
	}

	c->size += c->width * c->height * 4;
	content_set_ready(c);
	content_set_done(c);
	/* Done: update status bar */
	content_set_status(c, "");

	/* jmb: I'm really not sure that this should be here.
	 * The *_convert functions are for converting a content into a
	 * displayable format. They should not, however, do anything which
	 * could cause the content to be displayed; the content may have
	 * hidden visibility or be a fallback for an object; this
	 * information is not available here (nor is there any need for it
	 * to be).
	 * The specific issue here is that mng_display calls the display
	 * callbacks, which include nsmng_refresh. nsmng_refresh forces
	 * a content to be redrawn regardless of whether it should be
	 * displayed or not.
	 */
	/*	Start displaying
	*/
	status = mng_display(mng->handle);
	if ((status != MNG_NOERROR) && (status != MNG_NEEDTIMERWAIT)) {
		LOG(("Unable to start display (%i)", status));
		return nsmng_broadcast_error(mng, status) == NSERROR_OK;
	}
	bitmap_modified(mng->bitmap);

	/*	Optimise the plotting of JNG/PNGs
	*/
	mng->opaque_test_pending = true;
	bitmap_set_opaque(mng->bitmap, false);

	/* free associated memory */

	handle = mng->handle;

	mng_cleanup(&handle);

	mng->handle = NULL;

	return true;
}

static void nsmng_destroy(struct content *c)
{
	nsmng_content *mng = (nsmng_content *) c;

	assert (c != NULL);

	/*	Cleanup the MNG structure and release the canvas memory
	*/
	schedule_remove(nsmng_animate, c);

	if (mng->handle != NULL) {
		mng_handle handle = mng->handle;

		mng_cleanup(&handle);

		mng->handle = NULL;
	}

	if (mng->bitmap) {
		bitmap_destroy(mng->bitmap);
	}
}


static bool nsmng_redraw(struct content *c, struct content_redraw_data *data,
		const struct rect *clip, const struct redraw_context *ctx)
{
	nsmng_content *mng = (nsmng_content *) c;
	bool ret;
	bitmap_flags_t flags = BITMAPF_NONE;

	/* mark image as having been requested to display */
	mng->displayed = true;

	if ((mng->bitmap) && 
	    (mng->opaque_test_pending)) {
		bitmap_set_opaque(mng->bitmap, bitmap_test_opaque(mng->bitmap));
		mng->opaque_test_pending = false;
	}

	if (data->repeat_x)
		flags |= BITMAPF_REPEAT_X;
	if (data->repeat_y)
		flags |= BITMAPF_REPEAT_Y;

	ret = ctx->plot->bitmap(data->x, data->y, data->width, data->height,
			mng->bitmap, data->background_colour, flags);

	/* Check if we need to restart the animation */
	if ((mng->waiting) && 
	    (nsoption_bool(animate_images))) {
		nsmng_animate(c);
	}

	return ret;
}


static nserror nsmng_clone(const struct content *old, struct content **newc)
{
	nsmng_content *mng;
	nserror error;
	const char *data;
	unsigned long size;

	mng = calloc(1, sizeof(nsmng_content));
	if (mng == NULL)
		return NSERROR_NOMEM;

	error = content__clone(old, &mng->base);
	if (error != NSERROR_OK) {
		content_destroy(&mng->base);
		return error;
	}

	/* Simply replay create/process/convert */
	error = nsmng_create_mng_data(mng);
	if (error != NSERROR_OK) {
		content_destroy(&mng->base);
		return error;
	}

	data = content__get_source_data(&mng->base, &size);
	if (size > 0) {
		if (nsmng_process_data(&mng->base, data, size) == false) {
			content_destroy(&mng->base);
			return NSERROR_CLONE_FAILED;
		}
	}

	if (old->status == CONTENT_STATUS_READY ||
			old->status == CONTENT_STATUS_DONE) {
		if (nsmng_convert(&mng->base) == false) {
			content_destroy(&mng->base);
			return NSERROR_CLONE_FAILED;
		}
	}

	*newc = (struct content *) mng;

	return NSERROR_OK;
}

static void *nsmng_get_internal(const struct content *c, void *context)
{
	nsmng_content *mng = (nsmng_content *)c;

	return mng->bitmap;
}

static content_type nsmng_content_type(void)
{
	return CONTENT_IMAGE;
}

/* register handler for mng types */
static const content_handler nsmng_content_handler = {
	.create = nsmng_create,
	.process_data = nsmng_process_data,
	.data_complete = nsmng_convert,
	.destroy = nsmng_destroy,
	.redraw = nsmng_redraw,
	.clone = nsmng_clone,
	.get_internal = nsmng_get_internal,
	.type = nsmng_content_type,
	.no_share = false,
};

static const char *nsmng_types[] = {
        /* MNG types*/
	"image/mng",
	"image/x-mng",
	"video/mng",
	"video/x-mng",
};

CONTENT_FACTORY_REGISTER_TYPES(nsmng, nsmng_types, nsmng_content_handler);

/* register handler for jng and png types */
static const content_handler nsjpng_content_handler = {
	.create = nsmng_create,
	.process_data = nsmng_process_data,
	.data_complete = nsjpng_convert,
	.destroy = nsmng_destroy,
	.redraw = nsmng_redraw,
	.clone = nsmng_clone,
	.get_internal = nsmng_get_internal,
	.type = nsmng_content_type,
	.no_share = false,
};


static const char *nsjpng_types[] = {
        /* JNG types*/
	"image/jng",
	"image/x-jng",
        /* PNG types*/
	"image/png",
	"image/x-png"
};

CONTENT_FACTORY_REGISTER_TYPES(nsjpng, nsjpng_types, nsjpng_content_handler);

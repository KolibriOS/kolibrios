/*
 * Copyright © 2011 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Chris Wilson <chris@chris-wilson.co.uk>
 *
 */

#include <linux/kernel.h>
#include "../bitmap.h"

#include "sna.h"
#include "sna_render.h"
#include <memory.h>

#if DEBUG_STREAM
#undef DBG
#define DBG(x) ErrorF x
#endif

int sna_static_stream_init(struct sna_static_stream *stream)
{
	stream->used = 0;
	stream->size = 64*1024;

	stream->data = malloc(stream->size);
	return stream->data != NULL;
}

static uint32_t sna_static_stream_alloc(struct sna_static_stream *stream,
					uint32_t len, uint32_t align)
{
	uint32_t offset = ALIGN(stream->used, align);
	uint32_t size = offset + len;

	if (size > stream->size) {
/*
		do
			stream->size *= 2;
		while (stream->size < size);

		stream->data = realloc(stream->data, stream->size);
*/
        dbgprintf("%s: EPIC FAIL\n", __FUNCTION__);
        return 0;
	}

	stream->used = size;
	return offset;
}

uint32_t sna_static_stream_add(struct sna_static_stream *stream,
			       const void *data, uint32_t len, uint32_t align)
{
	uint32_t offset = sna_static_stream_alloc(stream, len, align);
	memcpy(stream->data + offset, data, len);
	return offset;
}

void *sna_static_stream_map(struct sna_static_stream *stream,
			    uint32_t len, uint32_t align)
{
	uint32_t offset = sna_static_stream_alloc(stream, len, align);
	return memset(stream->data + offset, 0, len);
}

uint32_t sna_static_stream_offsetof(struct sna_static_stream *stream, void *ptr)
{
	return (uint8_t *)ptr - stream->data;
}


struct kgem_bo *sna_static_stream_fini(struct sna *sna,
				       struct sna_static_stream *stream)
{
	struct kgem_bo *bo;

	DBG(("uploaded %d bytes of static state\n", stream->used));

	bo = kgem_create_linear(&sna->kgem, stream->used);
	if (bo && !kgem_bo_write(&sna->kgem, bo, stream->data, stream->used)) {
//       kgem_bo_destroy(&sna->kgem, bo);
		return NULL;
	}

	free(stream->data);
    LEAVE();
	return bo;
}

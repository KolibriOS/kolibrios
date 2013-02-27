#ifndef GEN4_SOURCE_H
#define GEN4_SOURCE_H

#include "compiler.h"

#include "sna.h"
#include "sna_render.h"

bool
gen4_channel_init_solid(struct sna *sna,
			struct sna_composite_channel *channel,
			uint32_t color);

bool
gen4_channel_init_linear(struct sna *sna,
			 PicturePtr picture,
			 struct sna_composite_channel *channel,
			 int x, int y,
			 int w, int h,
			 int dst_x, int dst_y);

#endif /* GEN4_SOURCE_H */

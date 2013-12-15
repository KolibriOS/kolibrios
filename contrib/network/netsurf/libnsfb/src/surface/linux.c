/*
 * Copyright 2012 Vincent Sanders <vince@simtec.co.uk>
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>

//#include <linux/fb.h>


#include "libnsfb.h"
#include "libnsfb_event.h"
#include "libnsfb_plot.h"
#include "libnsfb_plot_util.h"

#include "nsfb.h"
#include "plot.h"
#include "surface.h"
#include "cursor.h"



#define UNUSED(x) ((x) = (x))

#define FB_NAME "/dev/fb0"

struct lnx_priv {
    int fd;
};

static int linux_set_geometry(nsfb_t *nsfb, int width, int height, enum nsfb_format_e format)
{

    return -1;
}

static enum nsfb_format_e
format_from_lstate(void ) 
{
	

    return 0;
}

static int linux_initialise(nsfb_t *nsfb)
{
	return -1;
}

static int linux_finalise(nsfb_t *nsfb)
{
    return 0;
}

static bool linux_input(nsfb_t *nsfb, nsfb_event_t *event, int timeout)
{
    UNUSED(nsfb);
    UNUSED(event);
    UNUSED(timeout);
    return false;
}

static int linux_claim(nsfb_t *nsfb, nsfb_bbox_t *box)
{
    return 0;
}

static int linux_cursor(nsfb_t *nsfb, struct nsfb_cursor_s *cursor)
{
    return true;
}


static int linux_update(nsfb_t *nsfb, nsfb_bbox_t *box)
{
    return 0;
}

const nsfb_surface_rtns_t linux_rtns = {
    .initialise = linux_initialise,
    .finalise = linux_finalise,
    .input = linux_input,
    .claim = linux_claim,
    .update = linux_update,
    .cursor = linux_cursor,
    .geometry = linux_set_geometry,
};

NSFB_SURFACE_DEF(linux, NSFB_SURFACE_LINUX, &linux_rtns)

/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */

/* libnsfb plotter test program */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "libnsfb.h"
#include "libnsfb_plot.h"
#include "libnsfb_event.h"
#include "surface.h"

#include <menuet/os.h>

#define UNUSED(x) ((x) = (x))

extern const struct {
    unsigned int  width;
    unsigned int  height;
    unsigned int  bytes_per_pixel; /* 3:RGB, 4:RGBA */
    unsigned char pixel_data[132 * 135 * 4 + 1];
} nsglobe;

static bool
dump(nsfb_t *nsfb, const char *filename)
{
    int fd;

    if (filename  == NULL)
	return false;

    fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd < 0)
	return false;

    nsfb_dump(nsfb, fd);
    
    close(fd);

    return true;
}

int main(int argc, char **argv)
{
		extern nsfb_surface_rtns_t sdl_rtns;
		extern nsfb_surface_rtns_t ram_rtns;
	
	_nsfb_register_surface(NSFB_SURFACE_SDL, &sdl_rtns, "sdl");
	_nsfb_register_surface(NSFB_SURFACE_RAM, &ram_rtns, "ram");
	
	
    const char *fename;
    enum nsfb_type_e fetype;
    nsfb_t *nsfb;
    nsfb_t *bmp;
    nsfb_event_t event;
    int waitloop = 3;

    nsfb_bbox_t box;
    nsfb_bbox_t box2;
    nsfb_bbox_t box3;
    uint8_t *fbptr;
    int fbstride;
    const char *dumpfile = NULL;

	__menuet__debug_out("Init..\n");

    if (argc < 2) {
        fename="sdl";
    } else {
        fename = argv[1];
	if (argc >= 3) {
	    dumpfile = argv[2];
	}
    }

    fetype = nsfb_type_from_name(fename);
    if (fetype == NSFB_SURFACE_NONE) {
        fprintf(stderr, "Unable to convert \"%s\" to nsfb surface type\n", fename);
        return 1;
    }

    nsfb = nsfb_new(fetype);
    if (nsfb == NULL) {
        fprintf(stderr, "Unable to allocate \"%s\" nsfb surface\n", fename);
        return 2;
    }

    if (nsfb_init(nsfb) == -1) {
        fprintf(stderr, "Unable to initialise nsfb surface\n");
        nsfb_free(nsfb);
        return 4;
    }

	__menuet__debug_out("New ram surface...\n");

    bmp = nsfb_new(NSFB_SURFACE_RAM);
    
      if (bmp == NULL) {
       __menuet__debug_out("No ram surface :D ..\n");
       return 5;
    }
    __menuet__debug_out("Oh shit :D ..\n");
    nsfb_set_geometry(bmp, nsglobe.width, nsglobe.height, NSFB_FMT_ABGR8888);
    nsfb_init(bmp);
    nsfb_get_buffer(bmp, &fbptr, &fbstride);

    memcpy(fbptr, nsglobe.pixel_data, nsglobe.width * nsglobe.height * 4);

    /* get the geometry of the whole screen */
    box.x0 = box.y0 = 0;
    nsfb_get_geometry(nsfb, &box.x1, &box.y1, NULL);

    /* claim the whole screen for update */
    nsfb_claim(nsfb, &box);

    nsfb_plot_clg(nsfb, 0xffffffff);

    box3.x0 = 0;
    box3.y0 = 0;
    box3.x1 = 132;
    box3.y1 = 135;

    nsfb_plot_copy(bmp, &box3, nsfb, &box3);

    box3.x0 = 132;
    box3.y0 = 135;
    box3.x1 = box3.x0 + 264;
    box3.y1 = box3.y0 + 135;

    nsfb_plot_copy(bmp, &box3, nsfb, &box3);

    box3.x0 = 396;
    box3.y0 = 270;
    box3.x1 = box3.x0 + 264;
    box3.y1 = box3.y0 + 270;

    nsfb_plot_copy(bmp, &box3, nsfb, &box3);

    box2.x0 = 64;
    box2.y0 = 64;
    box2.x1 = 128;
    box2.y1 = 128;

    box3.x0 = 270;
    box3.y0 = 270;
    box3.x1 = box3.x0 + 64;
    box3.y1 = box3.y0 + 64;

    nsfb_plot_copy(nsfb, &box2, nsfb, &box3);

    nsfb_update(nsfb, &box);

    /* wait for quit event or timeout */
    while (waitloop > 0) {
	if (nsfb_event(nsfb, &event, 1000)  == false) {
	    break;
	}
	if (event.type == NSFB_EVENT_CONTROL) {
	    if (event.value.controlcode == NSFB_CONTROL_TIMEOUT) {
		/* timeout */
		waitloop--;
	    } else if (event.value.controlcode == NSFB_CONTROL_QUIT) {
		break;
	    }
	}
    }

    //dump(nsfb, dumpfile);

    nsfb_free(bmp);

    nsfb_free(nsfb);

    return 0;
}

/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */

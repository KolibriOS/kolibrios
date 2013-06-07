/*
 * Copyright 2013 SoUrcerer sourcerer@bk.ru
 *
 * This file is part of libnsfb, http://www.netsurf-browser.org/
 * Licenced under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 */

#include <stdbool.h>
#include <stdlib.h>

#include "libnsfb.h"
#include "libnsfb_event.h"
#include "libnsfb_plot.h"
#include "libnsfb_plot_util.h"

#include "nsfb.h"
#include "surface.h"
#include "palette.h"
#include "plot.h"
#include "cursor.h"


#include <menuet/os.h>



unsigned char * pixels;

 inline void f65(unsigned x, unsigned y, unsigned w, unsigned h, char *d)
{
asm("pusha");
asm ("nop"::"D"(0), "c"(w*65536+h), "d"(x*65536+y), "b"(d));
asm ("xor %eax, %eax");
asm ("movl %eax, %ebp");
asm ("pushl $32");
asm ("popl %esi");
asm ("int $0x40"::"a"(65));
asm("popa");
}


void kolibri_redraw(nsfb_t *nsfb){
	

 f65(0,0, nsfb->width, nsfb->height, pixels);

}

void kolibri_window_redraw(nsfb_t *nsfb){
	
 __menuet__window_redraw(1);
 __menuet__define_window(100,100,nsfb->width,nsfb->height,0x43000080,0x800000FF,0x000080);
 __menuet__write_text(3,3,0xFFFFFF,"Netsurf",7);
__menuet__debug_out("f65 is mighty!\n");

//here put image pixels! it's 32bpp
 f65(0,0, nsfb->width, nsfb->height, pixels);
 __menuet__window_redraw(2);
}



static bool 
kolibricopy(nsfb_t *nsfb, nsfb_bbox_t *srcbox, nsfb_bbox_t *dstbox)
{
   
    char *pixels = nsfb->surface_priv;
    nsfb_bbox_t allbox;
    struct nsfb_cursor_s *cursor = nsfb->cursor;

    nsfb_plot_add_rect(srcbox, dstbox, &allbox);

	int x,y,w,h;
    x = srcbox->x0;
    y = srcbox->y0;
    w = srcbox->x1 - srcbox->x0;
    h = srcbox->y1 - srcbox->y0;
    
    int tx, ty, tw, th;
    
    tx = dstbox->x0;
    ty = dstbox->y0;
    tw = dstbox->x1 - dstbox->x0;
    th = dstbox->y1 - dstbox->y0;
    
    char pst[255];
    sprintf (pst, "Src %d,%d %dx%d Dst %d,%d %dx%d \n", x,y,w,h,tx,ty,tw,th);
    __menuet__debug_out(pst);
    
    int px, py, pp;
    

    
    for (px=x; px<w; px++) 
		for (py=y;py<h;py++)
			for (pp=0; pp<4; pp++) {
				
				pixels[4*(px+tx)*nsfb->width+4*(py+ty)+pp]=pixels[4*px*nsfb->width+4*py+pp];
				
				
			}
    
    
    
    
	kolibri_redraw(nsfb);

    return true;

}

static int kolibri_set_geometry(nsfb_t *nsfb, int width, int height,
        enum nsfb_format_e format)
{
    if (nsfb->surface_priv != NULL)
        return -1; /* fail if surface already initialised */

    nsfb->width = width;
    nsfb->height = height;
    nsfb->format = format;
	
	pixels=(char *)malloc(width*height*4);
	
    /* select default sw plotters for format */
    select_plotters(nsfb);

    //nsfb->plotter_fns->copy = kolibricopy;

    return 0;
}
unsigned pz, pb;

static int kolibri_initialise(nsfb_t *nsfb)
{
    enum nsfb_format_e fmt;

pz=0;
pb=0;

__menuet__debug_out("Start UI\n");

    if (nsfb->surface_priv != NULL)
        return -1;

    /* sanity checked depth. */
    if ((nsfb->bpp != 32) ) {
		
		__menuet__debug_out("Wrong bpp\n");
        return -1; }

	
	        fmt = NSFB_FMT_XRGB8888;
    
    /* If we didn't get what we asked for, reselect plotters */
    if (nsfb->format != fmt) {
        nsfb->format = fmt;

        if (kolibri_set_geometry(nsfb, nsfb->width, nsfb->height,
                nsfb->format) != 0) {
					
					__menuet__debug_out("can't set geometry\n");
            return -1;
        }
    }

    nsfb->surface_priv = pixels;

    nsfb->ptr = pixels;
    nsfb->linelen = (nsfb->width * nsfb->bpp) / 8;
    
    __menuet__debug_out("Redraw\n");
    kolibri_redraw(nsfb);
    
    __menuet__set_bitfield_for_wanted_events(EVENT_REDRAW|EVENT_KEY|EVENT_BUTTON|EVENT_MOUSE_CHANGE);

    return 0;
}



static int kolibri_finalise(nsfb_t *nsfb)
{
    nsfb=nsfb;
    __menuet__sys_exit();
    return 0;
}


unsigned kol_mouse_posw()
{
unsigned error;
asm volatile ("int $0x40":"=a"(error):"a"(37), "b"(1));
return error;
}


unsigned kol_mouse_btn()
{
unsigned error;
asm volatile ("int $0x40":"=a"(error):"a"(37), "b"(2));
return error;
}



static bool kolibri_input(nsfb_t *nsfb, nsfb_event_t *event, int timeout)
{
    int got_event;
    

    nsfb = nsfb; /* unused */

       got_event = __menuet__check_for_event();

    if (got_event == 0) {
        return false;
    }

    event->type = NSFB_EVENT_NONE;

	 if (got_event==1) { //key pressed
    kolibri_window_redraw(nsfb);
	
	}

    if (got_event==2) { //key pressed
    event->type = NSFB_EVENT_KEY_UP;
	event->value.keycode = __menuet__getkey();
	return true;
	}
	
	if (got_event==3) { //key pressed
    if (__menuet__get_button_id()==1) kolibri_finalise(nsfb);
	return true;
	}
	
	if (got_event==6) { //key pressed
	unsigned z=kol_mouse_posw();
	unsigned b=kol_mouse_btn();
		
		
		if (pz!=z) {
			event->type = NSFB_EVENT_MOVE_ABSOLUTE;
			event->value.vector.x = (z&0xffff0000)>>16; //sdlevent.motion.x;
			event->value.vector.y = z&0xffff; //sdlevent.motion.y;
			event->value.vector.z = 0;
			pz=z;
			return true;
		}
		
		
		if (pb!=b) {
			unsigned t=b&1;
			if (t==0) {
				event->type = NSFB_EVENT_KEY_UP;
			    event->value.keycode = NSFB_KEY_MOUSE_1;
			} else {
				event->type = NSFB_EVENT_KEY_DOWN;
			    event->value.keycode = NSFB_KEY_MOUSE_1;
			}
			pb=b;		
			return true;
		}
		
	}

    /*

    case SDL_MOUSEBUTTONDOWN:
	event->type = NSFB_EVENT_KEY_DOWN;

	switch (sdlevent.button.button) {

	case SDL_BUTTON_LEFT:
	    event->value.keycode = NSFB_KEY_MOUSE_1;
	    break;

	case SDL_BUTTON_MIDDLE:
	    event->value.keycode = NSFB_KEY_MOUSE_2;
	    break;

	case SDL_BUTTON_RIGHT:
	    event->value.keycode = NSFB_KEY_MOUSE_3;
	    break;

	}
	break;

    case SDL_MOUSEBUTTONUP:
	event->type = NSFB_EVENT_KEY_UP;

	switch (sdlevent.button.button) {

	case SDL_BUTTON_LEFT:
	    event->value.keycode = NSFB_KEY_MOUSE_1;
	    break;

	case SDL_BUTTON_MIDDLE:
	    event->value.keycode = NSFB_KEY_MOUSE_2;
	    break;

	case SDL_BUTTON_RIGHT:
	    event->value.keycode = NSFB_KEY_MOUSE_3;
	    break;

	}
	break;

    case SDL_MOUSEMOTION:
	event->type = NSFB_EVENT_MOVE_ABSOLUTE;
	event->value.vector.x = sdlevent.motion.x;
	event->value.vector.y = sdlevent.motion.y;
	event->value.vector.z = 0;
	break;

    case SDL_QUIT:
	event->type = NSFB_EVENT_CONTROL;
	event->value.controlcode = NSFB_CONTROL_QUIT;
	break;

    case SDL_USEREVENT:
	event->type = NSFB_EVENT_CONTROL;
	event->value.controlcode = NSFB_CONTROL_TIMEOUT;
	break;

    }
	*/
    return true;
}


static int kolibri_claim(nsfb_t *nsfb, nsfb_bbox_t *box)
{
	/*
    if ((cursor != NULL) &&
        (cursor->plotted == true) &&
        (nsfb_plot_bbox_intersect(box, &cursor->loc))) {
        nsfb_cursor_clear(nsfb, cursor);
    } */
	return 0; //stub yet
}

static int kolibri_cursor(nsfb_t *nsfb, struct nsfb_cursor_s *cursor)
{
	return true; //stub yet
}



static int kolibri_update(nsfb_t *nsfb, nsfb_bbox_t *box)
{
    /*SDL_Surface *sdl_screen = nsfb->surface_priv;
    struct nsfb_cursor_s *cursor = nsfb->cursor;

    if ((cursor != NULL) &&
	(cursor->plotted == false)) {
        nsfb_cursor_plot(nsfb, cursor);
    }

    SDL_UpdateRect(sdl_screen,
                   box->x0,
                   box->y0,
                   box->x1 - box->x0,
                   box->y1 - box->y0);
  */
  
  //Ask for window redraw here!
  
	kolibri_redraw(nsfb);
    return 0;
}

const nsfb_surface_rtns_t kolibri_rtns = {
    .initialise = kolibri_initialise,
    .finalise = kolibri_finalise,
    .input = kolibri_input,
    .claim = kolibri_claim,
    .update = kolibri_update,
    .cursor = kolibri_cursor,
    .geometry = kolibri_set_geometry,
};

NSFB_SURFACE_DEF(kolibri, NSFB_SURFACE_KOLIBRI, &kolibri_rtns)

/*
 * Local variables:
 *  c-basic-offset: 4
 *  tab-width: 8
 * End:
 */

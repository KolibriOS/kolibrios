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

unsigned kol_scancodes()
{
unsigned error;
asm volatile ("int $0x40":"=a"(error):"a"(66), "b"(1), "c"(1));
return error;
}


void kolibri_redraw(nsfb_t *nsfb){
	

 f65(0,0, nsfb->width, nsfb->height, pixels);

}


unsigned kol_skin_h()
{
unsigned error;
asm volatile ("int $0x40":"=a"(error):"a"(48), "b"(4));
return error;
}

unsigned kol_area(char *data)
{
unsigned error;
asm volatile ("int $0x40":"=a"(error):"a"(9), "b"(data), "c"(0xffffffff));
return error;
}


void kolibri_window_redraw(nsfb_t *nsfb){
	
 __menuet__window_redraw(1);
 __menuet__define_window(100,100,nsfb->width+9,nsfb->height+kol_skin_h(),0x34000080,0x800000FF,"Netsurf");
 //__menuet__write_text(3,3,0xFFFFFF,"Netsurf",7);
//__menuet__debug_out("f65 is mighty!\n");

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
    
   // char pst[255];
  //  sprintf (pst, "Src %d,%d %dx%d Dst %d,%d %dx%d \n", x,y,w,h,tx,ty,tw,th);
   // __menuet__debug_out(pst);
    
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

	kol_scancodes(); 

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



int isup(int scan){
	return (scan&0x80)>>7;
}

int scan2key(int scan){
	int keycode=(scan&0x0FF7F);
	/* MAIN KB - NUMS */
	if (keycode == 0x02) return NSFB_KEY_1;
	if (keycode == 0x03) return NSFB_KEY_2;
	if (keycode == 0x04) return NSFB_KEY_3;
	if (keycode == 0x05) return NSFB_KEY_4;
	if (keycode == 0x06) return NSFB_KEY_5;
	if (keycode == 0x07) return NSFB_KEY_6;
	if (keycode == 0x08) return NSFB_KEY_7;
	if (keycode == 0x09) return NSFB_KEY_8;
	if (keycode == 0x0A) return NSFB_KEY_9;
	if (keycode == 0x0B) return NSFB_KEY_0;
	
	if (keycode == 0x10) return NSFB_KEY_q;
	if (keycode == 0x11) return NSFB_KEY_w;
	if (keycode == 0x12) return NSFB_KEY_e;
	if (keycode == 0x13) return NSFB_KEY_r;
	if (keycode == 0x14) return NSFB_KEY_t;
	if (keycode == 0x15) return NSFB_KEY_y;
	if (keycode == 0x16) return NSFB_KEY_u;
	if (keycode == 0x17) return NSFB_KEY_i;
	if (keycode == 0x18) return NSFB_KEY_o;
	if (keycode == 0x19) return NSFB_KEY_p;
	if (keycode == 0x1A) return NSFB_KEY_LEFTBRACKET;
	if (keycode == 0x1B) return NSFB_KEY_RIGHTBRACKET;
	
	if (keycode == 0x1E) return NSFB_KEY_a;
	if (keycode == 0x1F) return NSFB_KEY_s;
	if (keycode == 0x20) return NSFB_KEY_d;
	if (keycode == 0x21) return NSFB_KEY_f;
	if (keycode == 0x22) return NSFB_KEY_g;
	if (keycode == 0x23) return NSFB_KEY_h;
	if (keycode == 0x24) return NSFB_KEY_j;
	if (keycode == 0x25) return NSFB_KEY_k;
	if (keycode == 0x26) return NSFB_KEY_l;
	
	if (keycode == 0x2C) return NSFB_KEY_z;
	if (keycode == 0x2D) return NSFB_KEY_x;
	if (keycode == 0x2E) return NSFB_KEY_c;
	if (keycode == 0x2F) return NSFB_KEY_v;
	if (keycode == 0x30) return NSFB_KEY_b;
	if (keycode == 0x31) return NSFB_KEY_n;
	if (keycode == 0x32) return NSFB_KEY_m;
	
	if (keycode == 0x27) return NSFB_KEY_SEMICOLON;
	if (keycode == 0x28) return NSFB_KEY_QUOTEDBL;
	if (keycode == 0x2B) return NSFB_KEY_BACKSLASH;
	if (keycode == 0x33) return NSFB_KEY_COMMA;
	if (keycode == 0x34) return NSFB_KEY_PERIOD;
	if (keycode == 0x35) return NSFB_KEY_SLASH;
	if (keycode == 0x0C) return NSFB_KEY_MINUS;
	if (keycode == 0x0D) return NSFB_KEY_EQUALS;
	
	if (keycode == 0x0E) return NSFB_KEY_BACKSPACE;
	if (keycode == 0xE053) return NSFB_KEY_DELETE;
	if (keycode == 0x2A) return NSFB_KEY_LSHIFT;
	if (keycode == 0x36) return NSFB_KEY_RSHIFT;
	
	if (keycode == 0x1C) return NSFB_KEY_RETURN;
	
	if (keycode == 0xE04B) return NSFB_KEY_LEFT;
	if (keycode == 0xE04D) return NSFB_KEY_RIGHT;
	if (keycode == 0xE048) return NSFB_KEY_UP;
	if (keycode == 0xE050) return NSFB_KEY_DOWN;
	
	if (keycode == 0x3F) return NSFB_KEY_F5;
	
	if (keycode == 0x39) return NSFB_KEY_SPACE;
	if (keycode == 0x01) return NSFB_KEY_ESCAPE;
	
	if (keycode == 0x38) return NSFB_KEY_LALT;
	if (keycode == 0x1D) return NSFB_KEY_LCTRL;
	if (keycode == 0xE038) return NSFB_KEY_RALT;
	if (keycode == 0xE01D) return NSFB_KEY_RCTRL;
	
	
	if (keycode == 0xE047) return NSFB_KEY_HOME;
	if (keycode == 0xE04F) return NSFB_KEY_END;
	if (keycode == 0xE049) return NSFB_KEY_PAGEUP;
	if (keycode == 0xE051) return NSFB_KEY_PAGEDOWN;
	
	return NSFB_KEY_UNKNOWN;
	
}

int ispowerkey(int scan){
	return (scan&0xE000)>>15;
}


static bool kolibri_input(nsfb_t *nsfb, nsfb_event_t *event, int timeout)
{
    int got_event;
    static int scanfull=0;

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
    int scanz = __menuet__getkey();
    
    //char dbs[64];
    
    //__menuet__debug_out("KEY PRESSED\n");
    
   // sprintf (dbs, "FULLKEY BEFORE: F:%x\n", scanfull);
	//__menuet__debug_out(dbs);
	
	if (scanz==0xE0) {
		scanfull=0xE000;
		return true;
	} else {
		scanfull=scanfull+scanz;
	}
	
    //sprintf (dbs, "FULLKEY AFTER: F:%x\n", scanfull);
	//__menuet__debug_out(dbs);
    
    
	if (isup(scanfull)==1) {
	event->type = NSFB_EVENT_KEY_UP;} else {
	event->type = NSFB_EVENT_KEY_DOWN;}
		
	event->value.keycode = scan2key(scanfull);
	
	//sprintf (dbs, "KEY: %x F:%x %d %d\n", scanz, scanfull, isup(scanz), scan2key(scanz));
	//__menuet__debug_out(dbs);
	
	scanfull=0;
	
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

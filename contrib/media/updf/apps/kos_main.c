#include <menuet/os.h>
#define _WIN32
#include "fitz.h"
#include "mupdf.h"
#include "muxps.h"
#include "pdfapp.h"
#include "icons/allbtns.h"

// need to be a part of menuet/os.h
#define BT_DEL      0x80000000
#define BT_HIDE     0x40000000
#define BT_NOFRAME  0x20000000

#define evReDraw  1
#define evKey     2
#define evButton  3
#define evMouse   6
#define evNetwork 8

#define ASCII_KEY_LEFT  176
#define ASCII_KEY_RIGHT 179
#define ASCII_KEY_DOWN  177
#define ASCII_KEY_UP    178
#define ASCII_KEY_HOME  180
#define ASCII_KEY_END   181
#define ASCII_KEY_PGDN  183
#define ASCII_KEY_PGUP  184

#define ASCII_KEY_BS    8
#define ASCII_KEY_TAB   9
#define ASCII_KEY_ENTER 13
#define ASCII_KEY_ESC   27
#define ASCII_KEY_DEL   182
#define ASCII_KEY_INS   185
#define ASCII_KEY_SPACE 032

struct blit_call
{
   int dstx;       
   int dsty;
   int w;
   int h;

   int srcx;
   int srcy;
   int srcw;
   int srch;

   unsigned char *d;
   int   stride;
};

void blit(int dstx, int dsty, int w, int h, int srcx, int srcy,int srcw, int srch, int stride, char *d) //Вызов сисфункции Blitter
{
	struct blit_call image;
	image.dstx=dstx;
	image.dsty=dsty;
	image.w=w;
	image.h=h;
	image.srcx=srcx;
	image.srcy=srcy;
	image.srcw=srcw;
	image.srch=srch;
	image.stride=stride;
	image.d=d;
	asm ("int $0x40"::"a"(73),"b"(0),"c"(&image));
}


int __menuet__get_mouse_wheels(void)
{
    int val;
    asm ("int $0x40":"=a"(val):"a"(37),"b"(7));
    return val;
};

/*==== DATA ====*/

static char Title[1024] = "uPDF";
static char * filename = "/hd0/1/yand.pdf";
static pdfapp_t gapp;
char debugstr[256];
char do_not_blit=0;

#define TOOLBAR_HEIGHT 34
struct process_table_entry Form;

#define DOCUMENT_BORDER 0x979797
#define DOCUMENT_BG 0xABABAB

#define SCROLL_H 17

const char *help[] = {
	"Keys:",
	"  ",
	"PageUp   - go to previous page",
	"PageDown - go to next page",
	"Home     - go to first page",
	"End      - go to last page",
	"Down arrow - scroll current page down",
	"Up arrow   - scroll current page up",
	"+/- - zoom in/out",
	"[ or l - rotate page 90 deg to the left",
	"] or r - rotate page 90 deg to the right",
	"  ",
	"Press Escape to hide help",
	0
};

/*==== CODE ====*/


// not implemented yet
void wincursor(pdfapp_t *app, int curs) { }
void winhelp(pdfapp_t *app) { }
void winresize(pdfapp_t *app, int w, int h) { }
void windocopy(pdfapp_t *app) { }
void winopenuri(pdfapp_t *app, char *buf) { }
void winrepaintsearch(pdfapp_t *app) { }


void winwarn(pdfapp_t *app, char *msg)
{
	fprintf(stderr, "mupdf: %s\n", msg);
}


void winerror(pdfapp_t *app, fz_error error)
{
	fz_catch(error, "aborting");
	exit(1);
}


char *winpassword(pdfapp_t *app, char *filename)
{
	char *r = "";
	return r;
}


void wintitle(pdfapp_t *app, char *s)
{
	char* param = *(char**)0x1C;
	sprintf(Title,"%s - uPDF", strrchr(param, '/') + 1 );
}


void winreloadfile(pdfapp_t *app)
{
	//pdfapp_close(app);
	//pdfapp_open(app, filename, 0, 1);
}

void winclose(pdfapp_t *app)
{
	pdfapp_close(&gapp);
	__menuet__sys_exit();
}


void winrepaint(pdfapp_t *app)
{
	winblit(&gapp);
}


void winblit(pdfapp_t *app)
{
	signed short window_center, draw_w;

	if (do_not_blit) return;

	if (Form.client_width > gapp.image->w) window_center = (Form.client_width - gapp.image->w) / 2; else window_center = 0;

	if (gapp.image->n == 4)	 		 
		 	blit(window_center + Form.client_left, 
		 		Form.client_top + TOOLBAR_HEIGHT, 
		 		Form.client_width, 
		 		Form.client_height - TOOLBAR_HEIGHT, 
		 		gapp.panx, 
		 		gapp.pany, 
		 		gapp.image->w, 
		 		gapp.image->h, 
		 		gapp.image->w * gapp.image->n, 
		 		gapp.image->samples
		 	);
	else if (gapp.image->n == 2)
	{
		int i = gapp.image->w*gapp.image->h;
		unsigned char *color = malloc(i*4);
		if (color != NULL)
		{
			unsigned char *s = gapp.image->samples;
			unsigned char *d = color;
			for (; i > 0 ; i--)
			{
				d[2] = d[1] = d[0] = *s++;
				d[3] = *s++;
				d += 4;
			}
			blit(window_center + Form.client_left, 
				Form.client_top + TOOLBAR_HEIGHT, 
				Form.client_width, 
				Form.client_height - TOOLBAR_HEIGHT, 
		 		gapp.panx, 
		 		gapp.pany, 
				gapp.image->w, 
				gapp.image->h, 
				gapp.image->w * 4, 
				color
			);
			free(color);
		}
	} 
	if (gapp.image->h - gapp.pany < Form.client_height - TOOLBAR_HEIGHT)
	{
		if (gapp.image->w < Form.client_width) draw_w = gapp.image->w; else draw_w = Form.client_width;
		__menuet__bar(window_center, gapp.image->h - gapp.pany + TOOLBAR_HEIGHT, draw_w, 1, DOCUMENT_BORDER);
		__menuet__bar(window_center, gapp.image->h - gapp.pany + TOOLBAR_HEIGHT + 1, draw_w, Form.client_height - gapp.image->h - TOOLBAR_HEIGHT + gapp.pany - 1, DOCUMENT_BG);
	}
	DrawPagination();
}


int main (void)
{
	char ii, mouse_wheels_state;
	char* original_command_line = *(char**)0x1C;
	__menuet__debug_out(original_command_line);
	
	char buf[128];
	int resolution = 72;
	int pageno = 1;
	fz_accelerate();
	__menuet__debug_out("PDF init\n");
	pdfapp_init(&gapp);
	gapp.scrw = 600;
	gapp.scrh = 400;
	gapp.resolution = resolution;
	gapp.pageno = pageno;
	__menuet__debug_out("PDF Open\n");
	pdfapp_open(&gapp, original_command_line, 0, 0);
	__menuet__debug_out("PDF Opened\n");
	
 
	__menuet__debug_out("Inital paint\n");
	//if (gapp.image) gapp.shrinkwrap = 0;
	
	int butt, key, screen_max_x, screen_max_y;
	__menuet__get_screen_max(&screen_max_x, &screen_max_y);
	__menuet__set_bitfield_for_wanted_events(EVENT_REDRAW+EVENT_KEY+EVENT_BUTTON+EVENT_MOUSE_CHANGE);

 for(;;)
 {

	switch(__menuet__wait_for_event())
	{
		case evReDraw:
			__menuet__window_redraw(1);
			__menuet__define_window(screen_max_x / 2 - 350, screen_max_y / 2 - 300, 700, 600, 0x73000000, 0x800000FF, Title);
			__menuet__window_redraw(2);
			__menuet__get_process_table(&Form, PID_WHOAMI);
			if (Form.window_state > 2) continue; //fix rolled up
			Form.client_width++; //fix for Menuet kernel bug
			Form.client_height++; //fix for Menuet kernel bug
			DrawWindow();
			break;

		case evKey:
			key = __menuet__getkey(); 
			if (key==ASCII_KEY_ESC)  DrawWindow(); //close help 
			if (key==ASCII_KEY_PGDN) pdfapp_onkey(&gapp, ']');
			if (key==ASCII_KEY_PGUP) pdfapp_onkey(&gapp, '[');
			if (key==ASCII_KEY_HOME) pdfapp_onkey(&gapp, 'g');
			if (key==ASCII_KEY_END ) pdfapp_onkey(&gapp, 'G');
			if ((key=='[' ) || (key=='l')) pdfapp_onkey(&gapp, 'L');
			if ((key==']' ) || (key=='r')) pdfapp_onkey(&gapp, 'R');
			if (key==ASCII_KEY_DOWN ) PageScrollDown();
			if (key==ASCII_KEY_UP ) PageScrollUp();
			if (key=='-') PageZoomOut();
			if ((key=='=') || (key=='+')) PageZoomIn();
			break;

		case evButton:
			butt = __menuet__get_button_id();
			if(butt==1) __menuet__sys_exit();
			if(butt==10) ;//mag open file
			if(butt==11) PageZoomOut(); //magnify -
			if(butt==12) PageZoomIn(); //magnify +
			if(butt==13) //show help
			{
				__menuet__bar(0, TOOLBAR_HEIGHT, Form.client_width, Form.client_height - TOOLBAR_HEIGHT, 0xF2F2F2);	
				__menuet__write_text(20, TOOLBAR_HEIGHT + 20      , 0x90000000, "uPDF for KolibriOS v1.0", 0);
				__menuet__write_text(21, TOOLBAR_HEIGHT + 20      , 0x90000000, "uPDF for KolibriOS v1.0", 0);
				for (ii=0; help[ii]!=0; ii++) {
					__menuet__write_text(20, TOOLBAR_HEIGHT + 60 + ii * 15, 0x80000000, help[ii], 0);
				}
			}
			if(butt==14) pdfapp_onkey(&gapp, '['); //previous page
			if(butt==15) pdfapp_onkey(&gapp, ']'); //next page
			//if(butt==8) pdfapp_onkey(&gapp, 'j'); //move up
			//if(butt==9) pdfapp_onkey(&gapp, 'k'); //move down
			break;

		case evMouse:
			if (mouse_wheels_state = __menuet__get_mouse_wheels())
			{
				if (mouse_wheels_state==1) { PageScrollDown(); PageScrollDown(); }
				if (mouse_wheels_state==-1) { PageScrollUp();  PageScrollUp();   }
			}
			//sprintf (debugstr, "mouse_wheels_state: %d \n", mouse_wheels_state);
			//__menuet__debug_out(debugstr);
			//pdfapp_onmouse(&gapp, int x, int y, int btn, int modifiers, int state)
			break;
	}
  }
}



void DrawPageSides(void)
{
	short window_center;
	if (Form.client_width > gapp.image->w) window_center = (Form.client_width - gapp.image->w) / 2; else window_center = 0;
	if (gapp.image->w < Form.client_width)
	{
		__menuet__bar(0, TOOLBAR_HEIGHT, window_center-1, Form.client_height - TOOLBAR_HEIGHT, DOCUMENT_BG);
		__menuet__bar(window_center-1, TOOLBAR_HEIGHT, 1, Form.client_height - TOOLBAR_HEIGHT, DOCUMENT_BORDER);
		__menuet__bar(window_center + gapp.image->w, TOOLBAR_HEIGHT, 1, Form.client_height - TOOLBAR_HEIGHT, DOCUMENT_BORDER);
		__menuet__bar(window_center + gapp.image->w+1, TOOLBAR_HEIGHT, Form.client_width - window_center - gapp.image->w - 1, Form.client_height - TOOLBAR_HEIGHT, DOCUMENT_BG);
	}
}


void DrawPagination(void)
{
	char pages_display[12];
	short show_area_w = 65;
	short show_area_x = Form.client_width - show_area_w - 34;

	DrawToolbarButton(show_area_x - 26,5,26,24,4); //prev page
	DrawToolbarButton(show_area_x + show_area_w,5,26,24,5); //nex page 
	__menuet__bar(show_area_x,  5, show_area_w, 1, 0xA4A4A4);
	__menuet__bar(show_area_x,  6, show_area_w, 22, 0xF4F4F4);
	__menuet__bar(show_area_x, 28, show_area_w, 1, 0xA4A4A4);
	sprintf (pages_display, "%d/%d", gapp.pageno, gapp.pagecount);
	__menuet__write_text(show_area_x + show_area_w/2 - strlen(pages_display)*6/2, 14, 0x000000, pages_display, strlen(pages_display));
}


void DrawWindow(void)
{
	__menuet__bar(0, 0, Form.client_width, TOOLBAR_HEIGHT - 1, 0xe1e1e1); // bar on the top (buttons holder)
	__menuet__bar(0, TOOLBAR_HEIGHT - 1, Form.client_width, 1, 0x7F7F7F);
	DrawToolbarButton(8,5,26,24,0); //open_folder
	DrawToolbarButton(42,5,26,24,1); //magnify -
	DrawToolbarButton(67,5,26,24,2);  //magnify +
	DrawToolbarButton(Form.client_width - 160,5,26,24,3); //show help
	winblit(&gapp);
	DrawPageSides();
}


void DrawToolbarButton(int x, int y, int w, int h, char image_id)
{
	__menuet__make_button(x, y, w-1, h-1, 10 + image_id + BT_HIDE, 0);
	__menuet__putimage(x, y, w, h, image_id * 24 * 26 * 3 + toolbar_image);
}



/* Actions */

void PageScrollDown(void)
{
	if (gapp.image->h - gapp.pany - SCROLL_H < Form.client_height - TOOLBAR_HEIGHT)
	{
		pdfapp_onkey(&gapp, '.');
	}
	else {
		gapp.pany += SCROLL_H; 
		winblit(&gapp); 					
	}
}


void PageScrollUp(void)
{
	if (gapp.pany >= SCROLL_H) {
		gapp.pany -= SCROLL_H;
		winblit(&gapp);					
	}
	else {
		//need to avoid double repaint in future
		if (gapp.pageno == 1) return;
		do_not_blit = 1;
		pdfapp_onkey(&gapp, ',');
		do_not_blit = 0;
		gapp.pany = gapp.image->h - SCROLL_H - Form.client_height + TOOLBAR_HEIGHT;
		if (gapp.pany < 0) gapp.pany = 0;
		//sprintf (debugstr, "gapp.pany: %d \n", gapp.pany);
		//__menuet__debug_out(debugstr);
		winblit(&gapp);
	}
}


void PageZoomIn(void)
{
	pdfapp_onkey(&gapp, '+');
	DrawPageSides();
}


void PageZoomOut(void)
{
	pdfapp_onkey(&gapp, '-'); 
	DrawPageSides();
}



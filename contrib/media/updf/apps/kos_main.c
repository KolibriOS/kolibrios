/*==== INCLUDES ====*/

#include "fitz.h"
#include "mupdf.h"
#include "pdfapp.h"
#include "icons/allbtns.h"
#include "kolibri.h"


/*==== DATA ====*/

static char Title[1024] = "uPDF";
static pdfapp_t gapp;
char debugstr[256];
char do_not_blit=0;

#define TOOLBAR_HEIGHT 34
struct proc_info Form;

#define DOCUMENT_BORDER 0x979797
#define DOCUMENT_BG 0xABABAB

#define SCROLL_H 25

short show_area_w = 65;
short show_area_x;

char key_mode_enter_page_number;
int new_page_number;

static short window_center, draw_h, draw_w;

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
	"g - grayscale on/off",
	"  ",
	"Press Escape to hide help",
	0
};

/*==== CODE ====*/
// Prototypes //
void RunApp(char app[], char param[]);
void winblit(pdfapp_t *app);
void DrawPagination(void);
void HandleNewPageNumber(unsigned char key);
void ApplyNewPageNumber(void);
void DrawMainWindow(void);


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
	random();
}


void wintitle(pdfapp_t *app, char *s, char param[])
{
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
	exit(0);
}

void RunOpenApp(char name[])
{
	char cmd[250] = "*pdf* ";
	strcat(cmd, name);
	RunApp("/sys/lod", cmd);
}


void winrepaint(pdfapp_t *app)
{
	winblit(&gapp);
}


void winblit(pdfapp_t *app)
{

	if (do_not_blit) return;

	if (key_mode_enter_page_number==1) HandleNewPageNumber(0); else DrawPagination();

	if (Form.cwidth > gapp.image->w) window_center = (Form.cwidth - gapp.image->w) / 2; else window_center = 0;

	gapp.panx = 0;
	
	kos_blit(window_center + Form.cleft,
		Form.ctop + TOOLBAR_HEIGHT,
		Form.cwidth,
		Form.cheight - TOOLBAR_HEIGHT,
		gapp.panx, 
		gapp.pany, 
		gapp.image->w, 
		gapp.image->h, 
		gapp.image->w * gapp.image->n, // stride
		gapp.image->samples // image
	);
	
/*	
	void kos_blit(int dstx, int dsty, int w, int h, int srcx, int srcy, int srcw, int srch, int stride, char *d)
*/

}


void DrawPageSides(void)
{	
	if (gapp.image->h < Form.cheight - TOOLBAR_HEIGHT) {
		draw_h = gapp.image->h - gapp.pany; 
	} else {
		draw_h = Form.cheight - TOOLBAR_HEIGHT;
	}
	
	if (gapp.image->w < Form.cwidth) {
		window_center = (Form.cwidth - gapp.image->w) / 2;
		draw_w = gapp.image->w + 2;
		kol_paint_bar(0, TOOLBAR_HEIGHT, window_center-1, Form.cheight - TOOLBAR_HEIGHT, DOCUMENT_BG);
		kol_paint_bar(window_center-1, TOOLBAR_HEIGHT, 1, draw_h, DOCUMENT_BORDER);
		kol_paint_bar(window_center + gapp.image->w, TOOLBAR_HEIGHT, 1, draw_h, DOCUMENT_BORDER);
		kol_paint_bar(window_center + gapp.image->w+1, TOOLBAR_HEIGHT, Form.cwidth - window_center - gapp.image->w - 1, Form.cheight - TOOLBAR_HEIGHT, DOCUMENT_BG);
	} else {
		window_center = 1;
		draw_w = Form.cwidth;
	}
	
	kol_paint_bar(window_center - 1, gapp.image->h - gapp.pany + TOOLBAR_HEIGHT, draw_w, 1, DOCUMENT_BORDER);
	kol_paint_bar(window_center - 1, gapp.image->h - gapp.pany + TOOLBAR_HEIGHT + 1,
		draw_w, Form.cheight - gapp.image->h - TOOLBAR_HEIGHT + gapp.pany - 1, DOCUMENT_BG);
}


void GetNewPageNumber(void)
{
	new_page_number = gapp.pageno;
	key_mode_enter_page_number = 1;
	HandleNewPageNumber(0);
}

void HandleNewPageNumber(unsigned char key)
{
	char label_new_page[8];

	if ((key >= '0') && (key <= '9')) 
	{
		new_page_number = new_page_number * 10 + key - '0';
	}
	if (key == ASCII_KEY_BS)
	{
		new_page_number /= 10;
	}
	if (key == ASCII_KEY_ENTER)
	{
		ApplyNewPageNumber();
		return;
	}
	if (key==ASCII_KEY_ESC)
	{
		key_mode_enter_page_number = 0;
		DrawMainWindow();
		return;
	}

	itoa(new_page_number, label_new_page, 10);
	strcat(label_new_page, "_");
	kol_paint_bar(show_area_x,  6, show_area_w, 22, 0xFDF88E);
	kos_text(show_area_x + show_area_w/2 - strlen(label_new_page)*6/2, 14, 0x000000, label_new_page, strlen(label_new_page));

	if (new_page_number > gapp.pagecount) ApplyNewPageNumber();
}

void ApplyNewPageNumber(void)
{
	key_mode_enter_page_number = 0;
	gapp.pageno = new_page_number -1;
	pdfapp_onkey(&gapp, ']');
}

void DrawPagination(void)
{
	char pages_display[12];
	kol_paint_bar(show_area_x,  6, show_area_w, 22, 0xF4F4F4);
	sprintf (pages_display, "%d/%d", gapp.pageno, gapp.pagecount);
	kos_text(show_area_x + show_area_w/2 - strlen(pages_display)*6/2, 14, 0x000000, pages_display, strlen(pages_display));
}

void DrawToolbarButton(int x, char image_id)
{
	kol_btn_define(x, 5, 26-1, 24-1, 10 + image_id + BT_HIDE, 0);
	kol_paint_image(x, 5, 26, 24, image_id * 24 * 26 * 3 + toolbar_image);
}

void DrawMainWindow(void)
{
	kol_paint_bar(0, 0, Form.cwidth, TOOLBAR_HEIGHT - 1, 0xe1e1e1); // bar on the top (buttons holder)
	kol_paint_bar(0, TOOLBAR_HEIGHT - 1, Form.cwidth, 1, 0x7F7F7F);
	DrawToolbarButton(8,0); //open_folder
	DrawToolbarButton(42,1); //magnify -
	DrawToolbarButton(67,2);  //magnify +
	DrawToolbarButton(101,6); //rotate left
	DrawToolbarButton(126,7); //rotate right
	DrawToolbarButton(Form.cwidth - 160,3); //show help
	show_area_x = Form.cwidth - show_area_w - 34;
	DrawToolbarButton(show_area_x - 26,4); //prev page
	DrawToolbarButton(show_area_x + show_area_w,5); //nex page
	kol_btn_define(show_area_x-1,  5, show_area_w+1, 23, 20 + BT_HIDE, 0xA4A4A4);
	kol_paint_bar(show_area_x,  5, show_area_w, 1, 0xA4A4A4);
	kol_paint_bar(show_area_x, 28, show_area_w, 1, 0xA4A4A4);
	winblit(&gapp);
	DrawPageSides();
}


/* Actions */

void PageScrollDown(void)
{
	//pdfapp_onkey(&gapp, 'k'); //move down
	if (gapp.image->h - gapp.pany - SCROLL_H < Form.cheight - TOOLBAR_HEIGHT)
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
	//pdfapp_onkey(&gapp, 'j'); //move up
	if (gapp.pany >= SCROLL_H) {
		gapp.pany -= SCROLL_H;
		winblit(&gapp);					
	}
	else {
		//not very nice way of using do_not_blit, but it simple
		if (gapp.pageno == 1) return;
		do_not_blit = 1;
		pdfapp_onkey(&gapp, ',');
		do_not_blit = 0;
		gapp.pany = gapp.image->h - SCROLL_H - Form.cheight + TOOLBAR_HEIGHT;
		if (gapp.pany < 0) gapp.pany = 0;
		//sprintf (debugstr, "gapp.pany: %d \n", gapp.pany);
		//kol_board_puts(debugstr);
		winblit(&gapp);
	}
}

void RunApp(char app[], char param[])
{
	kol_struct70 r;
	r.p00 = 7;
	r.p04 = 0;
	r.p08 = param;
	r.p12 = 0;
	r.p16 = 0;
	r.p20 = 0;
	r.p21 = app;
	kol_file_70(&r);
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

void PageRotateLeft(void)
{
	pdfapp_onkey(&gapp, 'L');
	DrawPageSides();
}

void PageRotateRight(void)
{
	pdfapp_onkey(&gapp, 'R');
	DrawPageSides();
}

int main (int argc, char* argv[])
{
	char ii, mouse_wheels_state;
	
	// argv without spaces
	char full_argv[1024];
	for (int i = 1; i<argc; i++) {
		if (i != 1) strcat(full_argv, " ");
		strcat(full_argv, argv[i]);
	}
	
	if (argc == 1) {
		kol_board_puts("uPDF: no param set, showing OpenDialog...\n");
		RunOpenApp(argv[0]);
		exit(0);
	}

	kol_board_puts(full_argv);
	kol_board_puts("\n");
	
	char buf[128];
	int resolution = 72;
	int pageno = 1;
	fz_accelerate();
	kol_board_puts("PDF init...\n");
	pdfapp_init(&gapp);
	gapp.scrw = 600;
	gapp.scrh = 400;
	gapp.resolution = resolution;
	gapp.pageno = pageno;
	kol_board_puts("PDF Open...\n");
	pdfapp_open(&gapp, full_argv, 0, 0);
	kol_board_puts("PDF Opened!\n");
	wintitle(&gapp, 0, full_argv);
	 
	kol_board_puts("Inital paint\n");
	
	int butt, key, screen_max_x, screen_max_y;
	kos_screen_max(&screen_max_x, &screen_max_y);
	kol_event_mask(EVENT_REDRAW+EVENT_KEY+EVENT_BUTTON+EVENT_MOUSE_CHANGE);

	for(;;)
	{
		switch(kol_event_wait())
		{
			case evReDraw:
				// gapp.shrinkwrap = 2;
				kol_paint_start();
				kol_wnd_define(screen_max_x / 2 - 350-50+kos_random(50), 
				screen_max_y / 2 - 300-50+kos_random(50), 
				700, 600, 0x73000000, 0x800000FF, Title);
				kol_paint_end();
				kol_process_info(-1, (char*)&Form);
				
				if (Form.window_state & 4) continue; // if Rolled-up
				
				// Minimal size (700x600)
				if (Form.width < 700) kol_wnd_change(-1, -1, 700, -1);
				if (Form.height < 600)  kol_wnd_change(-1, -1, -1, 600);
				
				DrawMainWindow();
				break;

			case evKey:
				key = kos_get_key();
				if (key_mode_enter_page_number)
				{
					HandleNewPageNumber(key);
					break;
				}
				if (key==ASCII_KEY_ESC)  DrawMainWindow(); //close help 
				if (key==ASCII_KEY_PGDN) pdfapp_onkey(&gapp, ']');
				if (key==ASCII_KEY_PGUP) pdfapp_onkey(&gapp, '[');
				if (key==ASCII_KEY_HOME) pdfapp_onkey(&gapp, 'g');
				if (key==ASCII_KEY_END ) pdfapp_onkey(&gapp, 'G');
				if (key=='g' ) pdfapp_onkey(&gapp, 'c');
				if ((key=='[' ) || (key=='l')) PageRotateLeft();
				if ((key==']' ) || (key=='r')) PageRotateRight();
				if (key==ASCII_KEY_DOWN ) PageScrollDown();
				if (key==ASCII_KEY_UP ) PageScrollUp();
				if (key=='-') PageZoomOut();
				if ((key=='=') || (key=='+')) PageZoomIn();
				break;

			case evButton:
				butt = kol_btn_get();
				if(butt==1) exit(0);
				if(butt==10) RunOpenApp(argv[0]);
				if(butt==11) PageZoomOut(); //magnify -
				if(butt==12) PageZoomIn(); //magnify +
				if(butt==13) //show help
				{
					kol_paint_bar(0, TOOLBAR_HEIGHT, Form.cwidth, Form.cheight - TOOLBAR_HEIGHT, 0xF2F2F2);
					kos_text(20, TOOLBAR_HEIGHT + 20      , 0x90000000, "uPDF for KolibriOS v1.2", 0);
					kos_text(21, TOOLBAR_HEIGHT + 20      , 0x90000000, "uPDF for KolibriOS v1.2", 0);
					for (ii=0; help[ii]!=0; ii++) {
						kos_text(20, TOOLBAR_HEIGHT + 60 + ii * 15, 0x80000000, help[ii], 0);
					}
				}
				if(butt==14) pdfapp_onkey(&gapp, '['); //previous page
				if(butt==15) pdfapp_onkey(&gapp, ']'); //next page
				if(butt==16) PageRotateLeft();
				if(butt==17) PageRotateRight();
				if(butt==20) GetNewPageNumber();
				break;

			case evMouse:
				if (mouse_wheels_state = kos_get_mouse_wheels())
				{
					if (mouse_wheels_state==1) { PageScrollDown(); PageScrollDown(); }
					if (mouse_wheels_state==-1) { PageScrollUp();  PageScrollUp();   }
				}
				//sprintf (debugstr, "mouse_wheels_state: %d \n", mouse_wheels_state);
				//kol_board_puts(debugstr);
				//pdfapp_onmouse(&gapp, int x, int y, int btn, int modifiers, int state)
				break;
		}
	}
}

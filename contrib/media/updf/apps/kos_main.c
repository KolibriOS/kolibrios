#include<menuet/os.h>
#define _WIN32
#include "fitz.h"
#include "mupdf.h"
#include "muxps.h"
#include "pdfapp.h"
#include "icons/allbtns.h"



static char Title[] = "some title";
static char * filename = "/hd0/1/yand.pdf";
static pdfapp_t gapp;

void f65(unsigned x, unsigned y, unsigned w, unsigned h, char *d) //Вывод картинки
{
asm ("nop"::"c"(w*65536+h), "d"(x*65536+y), "b"(d));
asm ("xor %eax, %eax");
asm ("movl %eax, %ebp");
asm ("pushl $32");
asm ("popl %esi");
asm ("int $0x40"::"a"(65));

}

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


void wincursor(pdfapp_t *app, int curs)
{
	
}

void wintitle(pdfapp_t *app, char *s)
{

sprintf(Title,"uPDF: %s", s);	
}

void winhelp(pdfapp_t *app)
{
	
}

void winresize(pdfapp_t *app, int w, int h)
{
	//here should be something!!!
	
	
}


void windocopy(pdfapp_t *app)
{
}

void winreloadfile(pdfapp_t *app)
{
	pdfapp_close(app);


	pdfapp_open(app, filename, 0, 1);
}

void winopenuri(pdfapp_t *app, char *buf)
{
	/* here can be browser!
	char *browser = getenv("BROWSER");
	if (!browser)
		browser = "open";
	if (fork() == 0)
		execlp(browser, browser, buf, (char*)0);
		* */
		
}



void winclose(pdfapp_t *app)
{
	pdfapp_close(&gapp);
	__menuet__sys_exit();
}

void kol_paint_bar(unsigned x, unsigned y, unsigned w, unsigned h, unsigned c)
{
asm ("int $0x40"::"a"(13), "b"(x*65536+w), "c"(y*65536+h), "d"(c));
}


static void winblit(pdfapp_t *app)
{
	char yoba[32];
	int x0 = gapp.panx;
	int y0 = gapp.pany;
	int x1 = gapp.panx + gapp.image->w;
	int y1 = gapp.pany + gapp.image->h;
__menuet__debug_out(" Window blit\n");
__menuet__bar(0,0,598,370,0xababab); //background of pics
/*
	XSetForeground(xdpy, xgc, xbgcolor.pixel);
	fillrect(0, 0, x0, gapp.winh);
	fillrect(x1, 0, gapp.winw - x1, gapp.winh);
	fillrect(0, 0, gapp.winw, y0);
	fillrect(0, y1, gapp.winw, gapp.winh - y1);*/
	if (gapp.image->h-y0 > 0) {
	kol_paint_bar(0, gapp.image->h-y0, 590, 368, 0xEFEFEF);
}
	/*kol_paint_bar(x1, 0, gapp.winw - x1, gapp.winh, 0x00FF00);
	kol_paint_bar(0, 0, gapp.winw, y0, 0xFF0000);
	kol_paint_bar(0, y1, gapp.winw, gapp.winh - y1, 0xFFFF);*/
	

	/*XSetForeground(xdpy, xgc, xshcolor.pixel);
	fillrect(x0+2, y1, gapp.image->w, 2);
	fillrect(x1, y0+2, 2, gapp.image->h);*/
sprintf (yoba, "%d x %d, %d x %d \n", gapp.image->w, gapp.image->h, gapp.winw, gapp.winh);
__menuet__debug_out(yoba);


	if (gapp.image->n == 4)
		/*ximage_blit(xwin, xgc,
			x0, y0,
			gapp.image->samples,
			0, 0,
			gapp.image->w,
			gapp.image->h,
			gapp.image->w * gapp.image->n);*/
		//	f65(x0,y0+32,gapp.image->w,gapp.image->h,gapp.image->samples);
	 		 
		 	blit(6, 24, 588, 368, x0, y0,gapp.image->w, gapp.image->h, gapp.image->w * gapp.image->n, gapp.image->samples);
		 	
			
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
			/*ximage_blit(xwin, xgc,
				x0, y0,
				color,
				0, 0,
				gapp.image->w,
				gapp.image->h,
				gapp.image->w * 4); */
			//f65(x0,y0,gapp.image->w,gapp.image->h,color);
			blit(6, 24, 588, 368, x0, y0,gapp.image->w, gapp.image->h, gapp.image->w * 4, color);
			free(color);
		}
	}
 
 
 __menuet__bar(0,0,598,34,0xe1e1e1); // bar on the top (buttons holder)
 __menuet__bar(590,0,5,400,0xe1e1e1); // sidebar for scrolling 
 
 __menuet__make_button(8,5,26,24,7,0xe1e1e1); //(posirion x, position y, width, height, id, color) 
 __menuet__putimage(8,5,26,24,folder.pixel_data); 
   
 __menuet__make_button(42,5,26,24,6,0xe1e1e1); //magnify -
 __menuet__putimage(42,5,26,24,minus.pixel_data); 
 
 __menuet__make_button(68,5,26,24,5,0xe1e1e1); //magnify + 
 __menuet__putimage(68,5,26,24,plus.pixel_data); 

 
/* __menuet__make_button(100,0,20,20,10,0xe1e1e1); // rotate + 15 deg
 __menuet__write_text(105,3,0xFFFFFF,"p",1);
 
 __menuet__make_button(120,0,20,20,11,0xe1e1e1); // rotate - 15 deg
 __menuet__write_text(125,3,0xFFFFFF,"q",1); */
 
 __menuet__make_button(500,5,26,24,4,0xe1e1e1); //show help
 __menuet__putimage(500,5,26,24,help.pixel_data); 
 
 __menuet__make_button(534,5,24,24,2,0xe1e1e1); //prev page
 __menuet__putimage(534,5,24,24,prev.pixel_data); 
 
 __menuet__make_button(558,5,24,24,3,0xe1e1e1); //nex page 
 __menuet__putimage(558,5,24,24,next.pixel_data); 
 
  __menuet__make_button(573,34,17,17,8,0xe1e1e1); // move up
 __menuet__putimage(573,34,17,17,scrup.pixel_data); 
 
 __menuet__make_button(573,363,17,17,9,0xe1e1e1); // move down
 __menuet__putimage(573,363,17,17,scrdn.pixel_data); 
 
 
}

void paint(void)
{
 __menuet__window_redraw(1);
 //__menuet__define_window(10,10,600,400,0x64CFCFCF,0x800000FF,Title);
 __menuet__define_window(10,10,600,400,0x73CFCFCF,0x800000FF,Title);
 __menuet__bar(0,0,600,400,0xFFFFFF);
 winblit(&gapp);
 __menuet__window_redraw(2);
 }

void winrepaint(pdfapp_t *app)
{
	winblit(&gapp);
}

void winrepaintsearch(pdfapp_t *app)
{
	paint();
	//search!
}


int main (void)
{
	
	char* original_command_line = *(char**)0x1C;
	__menuet__debug_out(original_command_line);
	
	char buf[128];
	int resolution = 72;
		int pageno = 1;
		__menuet__debug_out("\nStarted\n");
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
	
	

 int i;
 int butt;
 
 __menuet__debug_out("Inital paint\n");
   pdfapp_onresize(&gapp, 600, 400);
 paint();
 for(;;)
 {

  i=__menuet__wait_for_event();
  butt = __menuet__get_button_id();
  if (gapp.image)
				{
					
						gapp.shrinkwrap = 0;
				}
  switch(i)
  {
   case 1:
    paint();
    
    continue;
   case 2:
    buf[0]=__menuet__getkey(); 
    pdfapp_onkey(&gapp, buf[0]);
    continue;
   case 3:
    if(butt==1) __menuet__sys_exit();//browse file
    if(butt==2) pdfapp_onkey(&gapp, '['); //previous page
    if(butt==3) pdfapp_onkey(&gapp, ']'); __menuet__debug_out("\nStarted\n"); //next page
    if(butt==4) pdfapp_onkey(&gapp, '?'); //show help window
    if(butt==5) pdfapp_onkey(&gapp, '+'); //magnify +
    if(butt==6) pdfapp_onkey(&gapp, '-'); //mag -
    if(butt==7) ;//mag open file
    if(butt==8) pdfapp_onkey(&gapp, 'j'); //move up
    if(butt==9) pdfapp_onkey(&gapp, 'k'); //move down
    if(butt==10) pdfapp_onkey(&gapp, 'a'); //rotate +15 deg
    if(butt==11) pdfapp_onkey(&gapp, 's'); //rotate -15deg
    continue;
  
  }
 }
 return 0;
}
  

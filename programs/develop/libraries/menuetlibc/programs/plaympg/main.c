#include<stdio.h>
#include<stdlib.h>
#include<menuet/os.h>
#include<mpeg.h>

ImageDesc I;
FILE * fmpeg=NULL;

char * vblit_buffer=NULL;
char * bitmap_buffer=NULL;
int line_width;

int blit_x_offs=0;
int blit_y_offs=0;

int win_size_x=0;
int win_size_y=0;

unsigned long inter_frame_delay;

char player_window_title[256];
char loaded_file_name[256];

enum {
 st_STOP=0,st_PLAYING=1,st_PAUSE=2,st_NOMOVIE=3
} play_state=st_NOMOVIE;

#define __convert_line(from,to) \
{ \
    int d0,d1,d2; \
    __asm__ __volatile__( \
	"1:\n\t" \
	"lodsw\n\t" \
	"stosw\n\t" \
	"lodsw\n\t" \
	"stosb\n\t" \
	"loop 1b" \
	:"=&c"(d0),"=&S"(d1),"=&D"(d2) \
	:"0"(line_width),"1"(from),"2"(to)); \
}

void convert_output_image(void)
{
 char * src=bitmap_buffer;
 char * dst=vblit_buffer;
 int i;
 for(i=0;i<I.Height;i++)
 {
  __convert_line(src,dst);
  src+=line_width;
  dst+=line_width;
 }
}

int reload_mpg(const char * fname)
{
 if(fmpeg) fclose(fmpeg);
 fmpeg=fopen(fname,"rb");
 if(!fmpeg) return -1;
 SetMPEGOption(MPEG_DITHER,FULL_COLOR_DITHER);
 OpenMPEG(fmpeg,&I);
 line_width=I.Width;
 vblit_buffer=(char *)realloc(vblit_buffer,I.Width*I.Height*3);
 bitmap_buffer=(char *)realloc(bitmap_buffer,I.Width*I.Height*4); 
 return 0;
}

void close_mpg(void)
{
 if(fmpeg) fclose(fmpeg);
 if(vblit_buffer) free(vblit_buffer);
 if(bitmap_buffer) free(bitmap_buffer);
 vblit_buffer=NULL;
 bitmap_buffer=NULL;
 fmpeg=NULL;
}

void rewind_mpg(void)
{
 RewindMPEG(fmpeg,&I);
}

int play_mpg_frame(void)
{
 if(!GetMPEGFrame(bitmap_buffer)) return -1;
 convert_output_image();
 __menuet__putimage(blit_x_offs,blit_y_offs,I.Width,I.Height,vblit_buffer);
 return 0;
}

void set_player_wnd_title(char * fname)
{
 int i;
 if(!fname || play_state==st_NOMOVIE)
  fname="No movie loaded";
 i=sprintf(player_window_title,"Menuet MPEG player - %s",fname);
 __menuet__write_text(4,4,0xffffff,player_window_title,i);
}

static char * player_buttons1[]={" ||  ","  >  ","  ## ","  /\\ ","  \\/ "};
static char * player_buttons2[]={"pause","play ","stop ","eject","load "};

#define BUTT_SIZE_X  (5*8)
#define BUTT_SIZE_Y  (2*12)

void paint_player_buttons(void)
{
 int xpos,ypos,i;
 xpos=5;
 ypos=20;
 for(i=0;i<5;i++)
 {
  __menuet__make_button(xpos,ypos,BUTT_SIZE_X,BUTT_SIZE_Y,i+2,0x40000000);
 }
}

void paint_player_window(void)
{
 __menuet__window_redraw(1);
 if(play_state==st_NOMOVIE)
 {
  win_size_x=40*8;
  win_size_y=200;
 } else {
  win_size_x=max(40*8,I.Width+20);
  win_size_y=max(200,I.Height+50);
 }
 __menuet__define_window(100,100,win_size_x,win_size_y,0x03000080,
                         0x800000FF,0x000080);
 set_player_wnd_title(loaded_file_name);
 if(play_state!=st_NOMOVIE)
 {
  blit_x_offs=10;
  blit_y_offs=40;
  __menuet__putimage(blit_x_offs,blit_y_offs,I.Width,I.Height,vblit_buffer);
 }
 paint_player_buttons();
 __menuet__window_redraw(2);
}

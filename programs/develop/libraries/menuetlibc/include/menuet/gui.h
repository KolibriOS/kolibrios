#ifndef __GUI_H
#define __GUI_H

#ifdef __cplusplus
extern "C" {
#endif

#include<menuet/os.h>

#define GUI_DX		10
#define GUI_DY		20

typedef struct __button_t
{
 int id;
 char * txt;
 int x,y;
 int xsz,ysz;
 void (* press_proc)(void);
} button_t;

typedef struct 
{
 int x,y;
 int max_len;
 __u32 back_color,text_color;
 int current;
 __u8 * buffer;
} input_line_t;

static inline void draw_button(button_t * b)
{
 int l;
 l=strlen(b->txt)<<3;
 l=(b->xsz-l)>>1;
 __menuet__make_button(b->x-2+10,b->y-2+20,b->xsz,b->ysz,b->id,0x808080);
 __menuet__write_text(b->x+l+10,b->y+((b->ysz-8)>>1)+20,0,b->txt,strlen(b->txt));
}

static inline void repaint_button_group(button_t * bt,int nr)
{
 int i;
 for(i=0;i<nr;i++) draw_button(&bt[i]);
}

static inline void check_button_click(button_t * bt,int nr,int clicked)
{
 int i;
 for(i=0;i<nr;i++) 
  if(bt[i].id==clicked)
  {
   if(bt[i].press_proc) bt[i].press_proc();
   return;
  }
}

static inline void draw_input_line(input_line_t * k)
{
 __menuet__bar(k->x+GUI_DX,k->y+GUI_DY,(k->max_len<<3)+10,8+5,k->back_color);
 if(k->current)
  __menuet__write_text(k->x+GUI_DX,k->y+GUI_DY,k->text_color,k->buffer,k->current);
}

static inline void handle_input_line(input_line_t * l)
{
 for(;;)
 {
  int k=__menuet__wait_for_event();
  switch(k)
  {
   case 1:
    WINDOW_PAINT_PROC();
    continue;
   case 2:
    k=__menuet__getkey();
    break;
   case 3:
    if(__menuet__get_button_id()==1) __menuet__sys_exit();
    continue;
  }
  if(!k) continue;
  if(k==13) return;
  if(k=='\b')
  {
   if(l->current) l->current--;
   l->buffer[l->current]='\0';
  } else {
   if(l->current>=l->max_len) continue;
   l->buffer[l->current]=(__u8)k;
   l->current++;
   l->buffer[l->current]='\0';
  }
  draw_input_line(l);
 }
}

static inline void outtextxy(int x,int y,__u32 tcol,__u32 bcol,char * txt)
{
 int l,m;
 m=l=strlen(txt);
 l<<=3;
 l+=4;
 __menuet__bar(GUI_DX+x,GUI_DY+y,l,10,bcol);
 __menuet__write_text(GUI_DX+x,GUI_DY+y,tcol,txt,m);
}

#ifdef __cplusplus
}
#endif

#endif

#include<menuet/os.h>
#include<stdlib.h>
#include"textcon.h"

console_t * consoles[MAX_CONSOLES];
console_t * visible_console;

static unsigned long color_conv_tbl[]=COLOR_CONV_B_2_D;

void init_consoles(void)
{
 int i;
 visible_console=NULL;
 for(i=0;i<MAX_CONSOLES;i++)
 {
  consoles[i]=(console_t *)malloc(sizeof(console_t));
  consoles[i]->text_color=7;
  consoles[i]->back_color=1;
  consoles[i]->id=i+1;
  consoles[i]->cur_x=0;
  consoles[i]->cur_y=0;
  consoles[i]->cur_visible=1;
  consoles[i]->cur_color=4;
  consoles[i]->esc_seq.esc[0]=0;
  consoles[i]->esc_seq.esc[1]=0;
  consoles[i]->esc_seq.esc[2]=0;
  consoles[i]->esc_seq.esc[3]=0;
  fill_empty_sem(&consoles[i]->io_lock);
  lcon_clrscr(consoles[i]);
 }
 visible_console=consoles[0];
 lcon_flush_console(visible_console);
}

void lcon_flush_console(console_t * con)
{
 int x,y;
 int ax,ay;
 char tbl[2];
 unsigned long c1,c2;
 if(con!=visible_console) return;
 tbl[1]='\0';
 for(y=0;y<NR_CHARS_Y;y++)
 {
  ay=CON_AT_Y+y*CHAR_SIZE_Y;
  for(x=0;x<NR_CHARS_X;x++)
  {
   ax=CON_AT_X+x*CHAR_SIZE_X;
   c1=color_conv_tbl[con->char_table[x][y].c_back];
   tbl[0]=(char)con->char_table[x][y].c_char&0xFF;
   if(con->cur_x==x && con->cur_y==y && con->cur_visible) 
   {
    c2=c1;
    c1=color_conv_tbl[con->cur_color];
   } else {
    c2=color_conv_tbl[con->char_table[x][y].c_color];
   }
   __menuet__bar(ax,ay,CHAR_SIZE_X,CHAR_SIZE_Y,c1);
   __menuet__write_text(ax,ay,c2,tbl,1);
  }
 }
}

void lcon_clrscr(console_t * con)
{
 int x,y;
 if(!con) return;
 for(y=0;y<NR_CHARS_Y;y++)
  for(x=0;x<NR_CHARS_X;x++)
  {
   con->char_table[x][y].c_char=' ';
   con->char_table[x][y].c_back=con->back_color;
   con->char_table[x][y].c_color=con->text_color;
  }
 lcon_flush_console(con);
}

void lcon_flushxy(console_t * con,int x,int y)
{
 char tbl[2];
 int ax,ay;
 unsigned long c1,c2;
 if(con!=visible_console) return;
 ay=CON_AT_Y+y*CHAR_SIZE_Y;
 ax=CON_AT_X+x*CHAR_SIZE_X;
 c1=color_conv_tbl[con->char_table[x][y].c_back];
 tbl[0]=(char)con->char_table[x][y].c_char&0xFF;
 if(con->cur_x==x && con->cur_y==y && con->cur_visible) 
 {
  c2=c1;
  c1=color_conv_tbl[con->cur_color];
 } else {
  c2=color_conv_tbl[con->char_table[x][y].c_color];
 }
 __menuet__bar(ax,ay,CHAR_SIZE_X,CHAR_SIZE_Y,c1);
 __menuet__write_text(ax,ay,c2,tbl,1);
}

void lcon_scroll(console_t * con,int update)
{
 int y,x;
 for(y=0;y<NR_CHARS_Y-1;y++)
 {
  for(x=0;x<NR_CHARS_X;x++)
  {
   memcpy(&con->char_table[x][y],&con->char_table[x][y+1],sizeof(char_info_t));
  }
 }
 for(x=0;x<NR_CHARS_X;x++)
 {
  con->char_table[x][NR_CHARS_Y-1].c_char=' ';
  con->char_table[x][NR_CHARS_Y-1].c_color=con->text_color;
  con->char_table[x][NR_CHARS_Y-1].c_back=con->back_color;
 }
 if(update) lcon_flush_console(con);
}

static int __con_update(console_t * con)
{
 int m;
 m=0;
 if(con->cur_x<0 || con->cur_y<0)
 {
  if(con->cur_x<0)
  {
   con->cur_x=0;
  }
  if(con->cur_y<0)
  {
   con->cur_y=0;
  }
  m=1;
 }
 if(con->cur_x>=NR_CHARS_X)
 {
  con->cur_y++;
  con->cur_x=0;
  m=1;
  if(con->cur_y>=NR_CHARS_Y)
  {
   con->cur_y=NR_CHARS_Y-1;
   lcon_scroll(con,0);
  }
 }
 return m;
}

void __lcon_putch_raw(console_t * con,char c)
{
 int tx,ty;
 __con_update(con);
 tx=con->cur_x;
 ty=con->cur_y;
 if(c=='\r')
 {
  con->cur_x=0;
 } else if(c=='\n')
 {
  con->cur_x=0;
  con->cur_y++;
 } else if(c=='\t')
 {
  con->cur_x=(con->cur_x+8)&~(8-1);
 } else if(c=='\b')
 {
  con->cur_x--;
  if(con->cur_x<0) 
  {
   con->cur_y--;
   con->cur_x=0;
  }
  if(con->cur_y<0)
  {
   con->cur_y=0;
  }
 } else {	
  con->char_table[con->cur_x][con->cur_y].c_char=c;
  con->char_table[con->cur_x][con->cur_y].c_color=con->text_color;
  con->char_table[con->cur_x][con->cur_y].c_back=con->back_color;
  con->cur_x++;
 }
 if(__con_update(con))
 {
  lcon_flush_console(con);
 } else {
  lcon_flushxy(con,tx,ty);
  if(con->cur_visible) lcon_flushxy(con,con->cur_x,con->cur_y);
 }
}

void lcon_gotoxy(console_t * con,int x,int y)
{
 int ox,oy;
 if(x<0 || x>=NR_CHARS_X || y<0 || y>=NR_CHARS_Y) return;
 ox=con->cur_x;
 oy=con->cur_y;
 con->cur_x=x;
 con->cur_y=y;
 if(con->cur_visible) 
 {
  lcon_flushxy(con,ox,oy);
  lcon_flushxy(con,x,y);
 }
}

void lcon_set_text_color(console_t * con,int color)
{
 con->text_color=color&0xFF;
}

void lcon_set_back_color(console_t * con,int color)
{
 con->back_color=color&0xFF;
}

void lcon_switch_to_console(int i)
{
 if(i<0 || i>=MAX_CONSOLES || !consoles[i]) return;
 visible_console=consoles[i];
 lcon_flush_console(visible_console);
}

unsigned char lcon_getcxy(console_t * con,int x,int y)
{
 if(x<0 || x>=NR_CHARS_X || y<0 || y>=NR_CHARS_Y) return;
 return con->char_table[x][y].c_char;
}

void lcon_putcxy(console_t * con,int x,int y,unsigned char c)
{
 if(x<0 || x>=NR_CHARS_X || y<0 || y>=NR_CHARS_Y) return;
 con->char_table[x][y].c_char=c;
 con->char_table[x][y].c_color=con->text_color;
 con->char_table[x][y].c_back=con->back_color;
}

static void lcon_set_attrib(console_t * con,unsigned char Att)
{
 static const unsigned ansi_to_vga[] = { 0, 4, 2, 6, 1, 5, 3, 7 };
 if(Att>=30 && Att<=37)
 {
  con->text_color=ansi_to_vga[Att-30];
 } else if(Att>=40 && Att<=48)
 {
  con->back_color=ansi_to_vga[Att-40];
 }
}

#undef isdigit
#define isdigit(c) ((c)>='0' && (c)<='9')

void __lcon_putch_help(console_t * con,unsigned char c)
{
 if(con->esc_seq.esc[0]==1)
 {
  if(c=='[')
  {
   con->esc_seq.esc[0]++;
   con->esc_seq.esc[1]=0;
   return;
  }
 } else if(con->esc_seq.esc[0]==2)
 {
  if(isdigit(c))
  {
   con->esc_seq.esc[1]=con->esc_seq.esc[1]*10+c-'0';
   return;
  } else if(c==';')
  {
   con->esc_seq.esc[0]++;
   con->esc_seq.esc[2]=0;
   return;
  } else if(c=='J')
  {
   if(con->esc_seq.esc[1]==2)
   {
    lcon_clrscr(con);
    lcon_gotoxy(con,0,0);
   }
  } else if(c=='m') lcon_set_attrib(con,con->esc_seq.esc[1]);
  con->esc_seq.esc[0]=0;
  return;
 } else if(con->esc_seq.esc[0]==3)
 {
  if(isdigit(c))
  {
   con->esc_seq.esc[2]=(con->esc_seq.esc[2]*10)+c-'0';
   return;
  } else if(c==';')
  {
   con->esc_seq.esc[0]++;
   con->esc_seq.esc[3]=0;
   return;
  } else if(c=='H')
  {
   int nx,ny;
   if(con->esc_seq.esc[2]<NR_CHARS_X)
    nx=con->esc_seq.esc[2]; else nx=con->cur_x;
   if(con->esc_seq.esc[1]<NR_CHARS_Y)
    ny=con->esc_seq.esc[1]; else ny=con->cur_y;
   lcon_gotoxy(con,nx,ny);
  } else if(c=='m')
  {
   lcon_set_attrib(con,con->esc_seq.esc[1]);
   lcon_set_attrib(con,con->esc_seq.esc[2]);
  }
  con->esc_seq.esc[0]=0;
  return;
 } else if(con->esc_seq.esc[0]==4)
 {
  if(isdigit(c))
  {
   con->esc_seq.esc[3]=con->esc_seq.esc[3]*10+c-'0';
   return;
  } else if(c=='m')
  {
   lcon_set_attrib(con,con->esc_seq.esc[1]);
   lcon_set_attrib(con,con->esc_seq.esc[2]);
   lcon_set_attrib(con,con->esc_seq.esc[3]);
  }
  con->esc_seq.esc[0]=0;
  return;
 }
 con->esc_seq.esc[0]=0;
 if(c==0x1B)
 {
  con->esc_seq.esc[0]=1;
  return;
 }
 __lcon_putch_raw(con,c);
}

void lcon_putch(console_t * con,char c)
{
 sem_lock(&con->io_lock);
 __lcon_putch_help(con,c);
 sem_unlock(&con->io_lock);
}

console_t * create_private_console(void)
{
 console_t * con=(console_t *)malloc(sizeof(console_t));
 if(!con) return NULL;
 con->id=MAX_CONSOLES|0x8000;
 con->cur_x=0;
 con->cur_y=0;
 con->cur_visible=0;
 con->cur_color=3;
 con->text_color=3;
 con->back_color=1;
 con->esc_seq.esc[0]=0;
 con->esc_seq.esc[1]=0;
 con->esc_seq.esc[2]=0;
 con->esc_seq.esc[3]=0;
 fill_empty_sem(&con->io_lock);
 lcon_clrscr(con);
 return con;
}

void free_private_console(console_t * con)
{
 if(con)
 {
  sem_lock(&con->io_lock); /* Wait for lock release and lock it */
  sem_unlock(&con->io_lock); /* Unlock immediately */
  free((void *)con); /* Free structure */
 }
}

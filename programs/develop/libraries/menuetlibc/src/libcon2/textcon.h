#ifndef __TEXTCON_H
#define __TEXTCON_H

#include<menuet/sem.h>

#define COLOR_CONV_B_2_D	{ \
 0x000000, \
 0x000080, \
 0x800000, \
 0x008080, \
 0x800000, \
 0x808000, \
 0x404040, \
 0x808080, \
 0x606060, \
 0x0000FF, \
 0x00FF00, \
 0x00FFFF, \
 0xFF0000, \
 0xFFFF00, \
 0x00FFFF, \
 0xFFFFFF, \
}

#define CHAR_SIZE_X		5
#define CHAR_SIZE_Y		8

#define NR_CHARS_X		80
#define NR_CHARS_Y		25

#define CON_AT_X		10
#define CON_AT_Y		25

typedef struct
{
 unsigned char c_char;
 unsigned char c_back;
 unsigned char c_color;
} char_info_t;

typedef struct
{
 int esc[4];
} esc_info_t;

typedef struct
{
 unsigned char text_color,back_color;
 char_info_t char_table[NR_CHARS_X][NR_CHARS_Y];
 int id;
 int cur_x,cur_y;
 int cur_visible;
 unsigned char cur_color;
 esc_info_t esc_seq;
 DECLARE_SEMAPHORE_S(io_lock);
} console_t;

#define MAX_CONSOLES		4

extern console_t * consoles[MAX_CONSOLES];
extern console_t * visible_console;

void init_consoles(void);
void lcon_clrscr(console_t * con);
void lcon_flush_console(console_t * con);
void lcon_flushxy(console_t * con,int x,int y);
void lcon_scroll(console_t * con,int update);
void lcon_putch(console_t * con,char c);
void lcon_gotoxy(console_t * con,int x,int y);
void lcon_set_text_color(console_t * con,int color);
void lcon_set_back_color(console_t * con,int color);
void lcon_switch_to_console(int i);
unsigned char lcon_getcxy(console_t * con,int x,int y);
void lcon_putcxy(console_t * con,int x,int y,unsigned char c);

#define _lcon_clrscr()		lcon_clrscr(visible_console)
#define _lcon_flush_console()	lcon_flush_console(visible_console)
#define _lcon_flushxy(x,y)	lcon_flushxy(visible_console,(x),(y))
#define _lcon_scroll()		lcon_scroll(visible_console,1)
#define _lcon_putch(c)		lcon_putch(visible_console,(char)(c))
#define _lcon_gotoxy(x,y)	lcon_gotoxy(visible_console,(x),(y))
#define _lcon_set_text_color(c) lcon_set_text_color(visible_console,(c)&(1+2+4+8))
#define _lcon_set_back_color(c) lcon_set_back_color(visible_console,(c)&(1+2+4+8))
#define _lcon_switch(i)		lcon_switch_to_console((i))
#define _lcon_getcxy(x,y)	lcon_getcxy(visible_console,(x),(y))
#define _lcon_putcxy(x,y,c)	lcon_putcxy(visible_console,(x),(y),(c))

console_t * create_private_console(void);
void free_private_console(console_t * con);

#endif

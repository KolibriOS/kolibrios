

#include "system/kolibri.h"
#include "system/stdlib.h"
#include "system/string.h"

#include "system/gblib.h"

#include "az3.h"
#include "az4.h"

//=====================================

#define scrw (8*32)
#define scrh (16+8*32)

//=====================================

typedef struct
{
char x;
char y;
} p_point;

//=====================================

char STR_TITLE[] = {"Piton 0.3.1"};

int mode;
char score[10];

char scr[scrw*scrh*3];
GB_BMP screen, az, azr;

char M[32][32];
p_point z[32*32];
p_point v;
p_point r;
int len;

unsigned color[] = {0xcccccc, 0xaa2222, 0x44bb, 0x7788aa};

 
//=====================================

void az_putc(unsigned char c, int x, int y)
{
if (c > 191)
	gb_image_set_t(&screen, x, y, &azr, (c-192)*8, 1, 8, 14, 0);
else 
	gb_image_set_t(&screen, x, y, &az, (c-' ')*8, 1, 8, 14, 0);
}

//=====================================

void az_puts(unsigned char *s, int x, int y)
{
unsigned i;
i = 0;
while (*(s+i))
	{
	az_putc(*(s+i), x+i*9, y);
	i++;
	}

}

//=====================================

az_puti(int n, int x, int y)
{
char c;
int i = 0;
do 
	{
	c = n % 10 + '0';
	az_putc(c, x-9*i, y);
	i++;
	}
	while ((n /= 10) > 0);
}

//=====================================

void clear()
{

int x, y;

for (y = 0; y < 32; y++)
	for (x = 0; x < 32; x++)
		M[x][y] = 0;

for (y = 0; y < 32; y++)
	{
	M[0][y] = 1;
	M[31][y] = 1;
	M[y][0] = 1;
	M[y][31] = 1;
	}
}

//=====================================

void put_z()
{
int i;

for (i = 0; i < len; i++)
	M[z[i].x][z[i].y] = 2;

}

//=====================================

void put_r()
{
M[r.x][r.y] = 3;
}

//=====================================

void rabbit_new()
{

for (;;)
	{
	r.x = rand()%29+1;
	r.y = rand()%29+1;

	if (0 == M[r.x][r.y])
		return;
	}
}
//=====================================

void game_start()
{

clear();

v.x = 1;
v.y = 0;

len = 2;

z[0].x = 16;
z[0].y = 16;

z[1].x = 15;
z[1].y = 16;

rabbit_new();
put_r();
put_z();

kol_sleep(30);
}

//=====================================

void press_space()
{
az_puts("нажмите Пробел", 10, 180);
az_puts("чтобы продолжить", 10, 195);
}

//=====================================

void screen_draw()
{

int x, y;

switch ( mode)
	{
	case 0:
		gb_bar(&screen, 0, 0, scrw, scrh, 0xbb);
		az_puts("П И Т О Н   0.3.1", 10, 60);
		az_puts("ремейк для ОС Колибри", 10, 120);
		az_puts("автор: А. Богомаз", 10, 135);
		press_space();
		break;
	

	case 1:
		gb_bar(&screen, 0, 0, scrw, 16, 0xbb);
		az_puts("ОЧКИ:", 10, 0);
		az_puti(len-2, 120, 0);
		for (y = 0; y < 32; y++)
			for (x = 0; x < 32; x++)
				gb_bar(&screen, x*8, y*8+16, 8, 8, color[ M[x][y] ]);

		break;

	case 2:
		gb_bar(&screen, 0, 0, scrw, scrh, 0xbb0000);
		az_puts("П А У З А", 10, 60);
		press_space();
		break;

	case 3:
		gb_bar(&screen, 0, 0, scrw, scrh, 0xee0000);
		az_puts("К О Н Е Ц    И Г Р Ы", 10, 60);
		kol_screen_wait_rr();
		kol_paint_image(0, 0, scrw, scrh, screen.bmp);
		kol_sleep(170);
		mode = 0;
		return;
	};

kol_screen_wait_rr();
kol_paint_image(0, 0, scrw, scrh, screen.bmp);

}

//=====================================

void wnd_draw()
{
kol_paint_start();
kol_wnd_define(280, 30, scrw+8, scrh+kol_skin_height()+4, 0x34888888, 0x34888888, STR_TITLE);
screen_draw();
kol_paint_end();
}

//=====================================

int piton_move()
{
int i;

for (i = len-1; i > 0; i--)
	{
	z[i].x = z[i-1].x;
	z[i].y = z[i-1].y;
	}

z[0].x += v.x;
z[0].y += v.y;

if ((1 == M[z[0].x][z[0].y])||(2 == M[z[0].x][z[0].y]))
	return -1;

if (3 == M[z[0].x][z[0].y])
	{
	rabbit_new();
	return 1;
	}


clear();
put_r();
put_z();

return 0;
}

//=====================================

void kol_main()
{

unsigned event, key;
int res;

srand(kol_system_time_get()<<8);

screen.bmp = scr;
screen.w = scrw;
screen.h = scrh;

az.bmp = AZ3;
az.w = 744;
az.h = 15;

azr.bmp = AZ4;
azr.w = 512;
azr.h = 15;

mode = 0;

for (;;)
	{
	kol_sleep(7);

	if ( 1 == mode)
		{
		res = piton_move();

		if (1 == res)
			len++;

		if (-1 == res)
			mode = 3;

		}
	
	screen_draw();
	event = kol_event_check();

	switch (event)
		{
		case 1:
			wnd_draw();
			break;

		case 2:
			key = (kol_key_get() & 0xff00)>>8;

			switch (mode)
				{

				case 0:
					if (32 == key)
						{
						mode = 1;
						game_start();
						}
					break;

				case 1:
					switch (key)
						{
						case 27:
							mode = 0;
							break;
						case 32:
							mode = 2;
							break;
						case 178:
							if (0 == v.y)
								{
								v.x = 0;
								v.y = -1;
								}
							break;
						case 177:
							if (0 == v.y)
								{
								v.x = 0;
								v.y = 1;
								}
							break;
						case 176:
							if (0 == v.x)
								{
								v.x = -1;
								v.y = 0;
								}
							break;
						case 179:
							if (0 == v.x)
								{
								v.x = 1;
								v.y = 0;
								}
							break;
						};
					break;

				case 2:
					if (32 == key)
						mode = 1;
					break;

				};

			break;

		case 3:
			if ( 1 == (kol_btn_get() & 0xff00)>>8 )
				kol_exit();
			break;

		};
	}

}

//=====================================


#include "system/kolibri.h"
#include "system/stdlib.h"
#include "system/string.h"

#include "system/gblib.h"

#include "car_01.h"
#include "car_02.h"
#include "donkey_01.h"
#include "donkey_02.h"
#include "az.h"

char STR_DONKEY[] = {"D O N K E Y  0.7"};

int start, paintbg, pause;
int dox, doy;
int drx, dry;

int drn, don;

int speed;


GB_BMP screen, font, car01, car02, donkey01, donkey02, az;


void az_putc(unsigned char c, int x, int y)
{
gb_image_set_t(&screen, x, y, &az, (c-'!')*11, 0, 11, 16, 0);
}



void az_puts(unsigned char *s, int x, int y)
{
unsigned i;
i = 0;
while (*(s+i))
	{
	az_putc(*(s+i), x+i*11, y);
	i++;
	}

}



void game_start()
{
drx = -1;
dry = 0;
doy = random(40)+130;

dox = random(1);
if (0 == dox)
	dox--;

drn = don = 0;
paintbg = 1;

speed = 0;

kol_sleep(30);
}


void screen_back()
{

gb_bar(&screen, 0, 0, 320, 200, 0x888888);

gb_bar(&screen, 6, 6, 97-6, 195-6, 0x488888);
gb_bar(&screen, 183, 6, 305-183, 195-6, 0x488888);

az_puts("Donkey", 6, 30);
az_puts("Driver", 189, 30);

az_putc (don+'0', 5, 48);
az_putc (drn/10+'0', 189, 48);

az_puts("Keys:", 189, 95);
az_puts("Space Bar", 189, 116);
az_puts("Enter", 189, 132);
az_puts("Esc", 189, 148);


gb_line(&screen, 100, 0, 100, 199, 0xffffff);
gb_line(&screen, 180, 0, 180, 199, 0xffffff);

paintbg = 0;
}



void screen_draw()
{

int y;

if (-1 == pause)
	{
	screen_back();

	gb_bar(&screen, 4, 20, 255, 60, 0xee0000);
	az_puts ("P A U S E", 5, 30);
	az_puts ("Press Enter to continue", 5, 60);

	kol_screen_wait_rr();
	kol_paint_image( 0, 0, 320, 200, screen.bmp);

	paintbg = 1;
	return;
	}

if (don > 9)
	{
	gb_bar(&screen, 0, 0, 320, 200, 0x880000);
	az_puts ("G A M E    O V E R", 5, 30);

	kol_screen_wait_rr();
	kol_paint_image( 0, 0, 320, 200, screen.bmp);

	kol_sleep(150);
	game_start();
	start = 1;
	}

if (drn > 99)
	{
	gb_bar(&screen, 0, 0, 320, 200, 0x88);
	az_puts ("CONGRATULATIONS !!!", 5, 30);

	kol_screen_wait_rr();
	kol_paint_image( 0, 0, 320, 200, screen.bmp);

	kol_sleep(150);
	game_start();
	start = 1;
	}

if (drn > 9)
	speed = 1;

if (drn > 84)
	speed = 2;


if (start)
	{
	if (!paintbg)
		return;

	gb_bar(&screen, 0, 0, 320, 200, 0x55);

	az_puts ( STR_DONKEY, 5, 30);
	az_puts ("remake for KolibriOS", 5, 78);
	az_puts ("by Albom", 5, 78+16);

	az_puts ("Press Space Bar", 5, 140);
	az_puts ("to continue", 5, 156);


	kol_screen_wait_rr();
	kol_paint_image( 0, 0, 320, 200, screen.bmp);
	paintbg = 0;
	return;
	}

if (paintbg)
	screen_back();
else 
	gb_bar(&screen, 102, 0, 180-102, 200, 0x888888);

for (y=10; y<180; y+=20)
	gb_line(&screen, 140, y-10*(doy%2), 140, y+10-10*(doy%2), 0xffffff);

doy-=5;
if (doy < -50)
	{

	dox = random(1);
	if (0 == dox)
		dox--;

	doy = random(40)+130;
	dry += 10;
	drn++;
	if (0 == drn%10)
		dry = 0;
	paintbg = 1;
	}

if ((drx == dox)&&(dry > doy-15)&&(dry < doy+15))
	{
	don++;
	dry = 0;
	drn /= 10;
	drn *= 10; 
	for (y = 0; y<2; y++)
		{
		kol_sleep(8);
		screen_back();
		az_puts("BOOM!", 6, 120);

		gb_image_set_t(&screen, 140-13-(y+1)*20, 120-dry+(y+1)*20, &car01, 0, 0, 13, 34, 0x888888);
		gb_image_set_t(&screen, 120+(y+3)*20, 120-dry+(y+1)*20, &car02, 0, 0, 13, 34, 0x888888);

		gb_image_set_t(&screen, 140-17-(y+1)*20, 120-(y+1)*20, &donkey01, 0, 0, 17, 17, 0x888888);
		gb_image_set_t(&screen, 140+(y+3)*20, 120-(y+1)*20, &donkey02, 0, 0, 13, 17, 0x888888);

		kol_screen_wait_rr();
		kol_paint_image(0, 0, 320, 200, screen.bmp);
		}

	dox = random(1);
	if (0 == dox)
		dox--;

	doy = random(40)+130;
	paintbg = 1;
	}

gb_image_set_t(&screen, 140-13-drx*20, 130-dry, &car01, 0, 0, 13, 34, 0x888888);
gb_image_set_t(&screen, 140-drx*20, 130-dry, &car02, 0, 0, 13, 34, 0x888888);

gb_image_set_t(&screen, 140-17-dox*20, 130-doy, &donkey01, 0, 0, 17, 17, 0x888888);
gb_image_set_t(&screen, 140-dox*20, 130-doy, &donkey02, 0, 0, 13, 17, 0x888888);

kol_screen_wait_rr();
kol_paint_image(0, 0, 320, 200, screen.bmp);

}



void wnd_draw()
{
kol_paint_start();
kol_wnd_define(280, 200, 328, 204+kol_skin_height(), 0x34888888);
kol_wnd_caption(STR_DONKEY);
screen_draw();
kol_paint_end();
}




void kol_main()
{

unsigned event;
unsigned key;

srand(kol_system_time_get()<<8);
start = 1;
paintbg = 1;

pause = 1;

speed = 0;

screen.bmp = malloc(320*200*3);
screen.w = 320;
screen.h = 200;

car01.bmp = CAR_01;
car01.w = 13;
car01.h = 34;

car02.bmp = CAR_02;
car02.w = 13;
car02.h = 34;

donkey01.bmp = DONKEY_01;
donkey01.w = 17;
donkey01.h = 17;

donkey02.bmp = DONKEY_02;
donkey02.w = 13;
donkey02.h = 17;

az.bmp = AZ;
az.w = 1012;
az.h = 16;


wnd_draw();

for (;;)
	{

	kol_sleep(6-speed);
	screen_draw();
	event = kol_event_check();

	switch (event)
		{
		case 1:
			paintbg = 1;
			wnd_draw();
			break;

		case 2:
			key = (kol_key_get() & 0xff00)>>8;
			if (start && (32==key))
				{
				start = 0;
				game_start();
				screen_draw();
				break;
				}

			if (27 == key)
				{
				if (-1 == pause)
					break;
				start = 1;
				paintbg = 1;
				break;
				}

			if (32 == key)
				{
				if (-1 == pause)
					break;
				drx *= -1;
				screen_draw();
				}

			if ((!start) && (13 == key))
				{
				pause *= -1;
				screen_draw();
				}


			break;

		case 3:
			if ( 1 == (kol_btn_get() & 0xff00)>>8 )
				kol_exit();
			break;

		};
	}

}

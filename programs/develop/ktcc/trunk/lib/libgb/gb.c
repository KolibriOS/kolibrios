
#include <kos/gb.h>

//=========================

void gb_pixel_set(GB_BMP *b, int x, int y, unsigned c)
{
// поставить точку

unsigned s;

if ((x+1 > b->w )||(y+1 > b->h))
	return;

if ((x < 0)||(y < 0))
	return;

s = 3*( y*(b->w) + x );

*( b -> bmp + s ) = c & 0xff;
*( b -> bmp + s + 1) = (c >> 8) & 0xff;
*( b -> bmp + s + 2) = (c >> 16)& 0xff;

}

//=========================

int gb_pixel_get(GB_BMP *b, int x, int y, unsigned *c)
{
// получить точку

unsigned red, green, blue, s;

if ((x < 0) || (y < 0))
	return 0;

if ((x+1 > b->w )||(y+1 > b->h))
	return 0;

s = 3*( y*(b->w) + x );

blue  = *( b -> bmp + s );
green = *( b -> bmp + s + 1);
red   = *( b -> bmp + s + 2);

*c = ((red << 16) & 0xff0000) | ((green << 8) & 0xff00) | (blue & 0xff);

return 1;
}

//=========================

void gb_line(GB_BMP *b, int x1, int y1, int x2, int y2, unsigned c)
{
// линия заданного цвета с использованием 
// алгоpитма Брезенхэма

int t, dist;
int xerr=0, yerr=0, delta_x, delta_y;
int incx, incy;

// вычисление pасстояния в обоих напpавлениях
delta_x = x2 - x1;
delta_y = y2 - y1;

// опpеделение напpавления шага,
// шаг вычисляется либо по веpтикальной, либо гоpизонтальной
// линии
if (delta_x > 0) 
	incx = 1;
else  
	if (0 == delta_x) 
		incx = 0;
	else
		incx = -1;

if (delta_y > 0) 
	incy = 1;
else
	if ( 0 == delta_y) 
		incy = 0;
	else
		incy = -1;

// опpеделение наибольшего pасстояния
if (delta_x < 0)
	delta_x *= -1;
if (delta_y < 0)
	delta_y *= -1;

if (delta_x > delta_y) 
	dist = delta_x;
else 
	dist = delta_y;


// вычеpчивание линии
for (t = 0; t <= dist+1; t++)
	{
	gb_pixel_set(b, x1, y1, c);
	xerr+=delta_x;
	yerr+=delta_y;

	if (xerr > dist) 
		{
		xerr -= dist;
		x1 += incx;
		}

	if (yerr > dist) 
		{
		yerr -= dist;
		y1 += incy;
		}

	}
}

//=========================

void gb_rect(GB_BMP *b, int x, int y, int w, int h, unsigned c)
{
// прямоугольник

gb_line (b, x,     y,     x+w-1, y,     c);
gb_line (b, x,     y+h-1, x+w-1, y+h-1, c);
gb_line (b, x,     y,     x,     y+h-1, c);
gb_line (b, x+w-1, y,     x+w-1, y+h-1, c);
}

//=========================

void gb_circle(GB_BMP *b, int x, int y, int r, unsigned c)
{

int _x;
int _y;
int d;

_x = 0;
_y = r;
d = 3-2*r;
while( _y >= _x)
	{
	gb_pixel_set(b,  _x + x,  _y + y, c);
	gb_pixel_set(b,  _x + x, -_y + y, c);
	gb_pixel_set(b, -_x + x,  _y + y, c);
	gb_pixel_set(b, -_x + x, -_y + y, c);
	gb_pixel_set(b,  _y + x,  _x + y, c);
	gb_pixel_set(b,  _y + x, -_x + y, c);
	gb_pixel_set(b, -_y + x,  _x + y, c);
	gb_pixel_set(b, -_y + x, -_x + y, c);
	if( d<0 )
		d = d+4*_x+6;
        else
		{
		d = d+4*(_x-_y)+10;
		_y--;
		}
	_x++;
	}

}

//=========================

void gb_bar(GB_BMP *b, int x, int y, int w, int h, unsigned c)
{
// закрашенный прямоугольник

unsigned s;
int i, j;

if ((x > b->w)||(y > b->h))
	return;

for (j = 0; j < w; j++)
for (i = 0; i < h; i++)
//	gb_pixel_set(b, x+j, y+i, c);
	{
	s = 3*( (y+i)*(b->w) + x + j );

	*( b -> bmp + s ) = c & 0xff;
	*( b -> bmp + s + 1) = (c >> 8) & 0xff;
	*( b -> bmp + s + 2) = (c >> 16)& 0xff;

	}
}

//=========================

void gb_image_set(GB_BMP *b_dest, int x_d, int y_d, GB_BMP *b_src, int x_s, int y_s, int w, int h)
{
// вывод изображения

int x, y;
unsigned d;

if ((x_d > b_dest->w)||(y_d > b_dest->h))
	return;

if ((x_s > b_src->w)||(y_s > b_src->h))
	return;

for (y = 0; y < h; y++)
	for (x = 0; x < w; x++)
		if ( gb_pixel_get(b_src, x_s+x, y_s+y, &d) )
			gb_pixel_set(b_dest, x_d+x, y_d+y, d);


}

//=========================

void gb_image_set_t(GB_BMP *b_dest, int x_d, int y_d, GB_BMP *b_src, int x_s, int y_s, int w, int h, unsigned c)
{
// вывод изображения с прозрачным цветом

int x, y;
unsigned d;

if ((x_d > b_dest->w)||(y_d > b_dest->h))
	return;

if ((x_s > b_src->w)||(y_s > b_src->h))
	return;

for (y = 0; y < h; y++)
	for (x = 0; x < w; x++)
		if ( gb_pixel_get(b_src, x_s+x, y_s+y, &d) )
			if (c != d)
				gb_pixel_set(b_dest, x_d+x, y_d+y, d);


}

//=========================

#define NULL ((void*)0)

typedef struct
{
void	*name;
void	*function;
} export_t;

//=========================

char szGb_pixel_set[]   =  "gb_pixel_set";
char szGb_pixel_get[]   =  "gb_pixel_get";
char szGb_line[]        =  "gb_line";
char szGb_rect[]        =  "gb_rect";
char szGb_bar[]         =  "gb_bar";
char szGb_circle[]      =  "gb_circle";
char szGb_image_set[]   =  "gb_image_set";
char szGb_image_set_t[] =  "gb_image_set_t";

export_t EXPORTS[] =
{
{ szGb_pixel_set,   (void*) gb_pixel_set},
{ szGb_pixel_get,   (void*) gb_pixel_get},
{ szGb_line,        (void*) gb_line},
{ szGb_rect,        (void*) gb_rect},
{ szGb_bar,         (void*) gb_bar},
{ szGb_circle,      (void*) gb_circle},
{ szGb_image_set,   (void*) gb_image_set},
{ szGb_image_set_t, (void*) gb_image_set_t},
{ NULL, NULL },
};


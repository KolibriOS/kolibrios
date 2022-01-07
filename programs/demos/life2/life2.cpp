#include <kolibri.h>
#include <kos_heap.h>
#include <kos_file.h>
#include <load_lib.h>
#include <l_proc_lib.h>
#include "lifegen.h"
#include "life_bmp.h"

using namespace Kolibri;

char library_path[2048];

OpenDialog_data ofd;
unsigned char procinfo[1024];
char plugin_path[4096], filename_area[256];
od_filter filter1 = { 8, "LIF\0\0" };

namespace Kolibri{
	char CurrentDirectoryPath[2048];
}

void __stdcall DrawWindow()
{
	asm{
		push ebx
		mcall SF_REDRAW,SSF_BEGIN_DRAW
	}
	//KolibriOnPaint();
	asm{
		mcall SF_REDRAW,SSF_END_DRAW
		pop ebx
	}
}

void __stdcall OneGeneration(int w, int h, void *dest, const void *src, int flag);

struct GenerateParam
{
	unsigned int gps;
	unsigned int paint_time, time, count;
	bool stop;
	int paint;
	double speed;
};

struct AxisParam
{
	unsigned int win;
	int p;
	double shift;
};

struct MouseParam
{
	int hit_x, hit_y, last_x, last_y;
	int button, hit_type;

	enum {HitNull = 0, HitLine, HitCircle, HitScroll};
};

struct MenuButtonParam
{
	int left, size, border;
	bool check;
	const unsigned char *bitmap;

	int Left() const {return left;}
	int Right() const {return left + size;}
};

const int MenuDig = 10;

struct MenuParam
{
	enum {Size = 20, NButton = 14};

	bool draw;
	int pressed, current, edit;
	int edit_index, edit_num[2], edit_num_max;
	MenuButtonParam button[NButton];
	const unsigned char *digit[MenuDig];
};

struct TimeGeneration
{
	unsigned int t, g;
};

enum MenuItem {MenuIHide, MenuIClear, MenuIOpen, MenuIAbout, MenuIExit,
				MenuIGenerate, MenuIRandom, MenuIVCircle, MenuIHCircle,
				MenuILine, MenuIScroll, MenuIWinSize, MenuISize, MenuISpeed};

enum {PaintWNull = 0, PaintWPole = 1, PaintWMenuBorder = 2, PaintWMenu = 6,
		PaintWSpeed = 8, PaintWAll = 15, PaintWFast = 64, PaintWNow = 128};

enum {TimeGenLength = 500};

unsigned char *life_data = 0, *picture = 0;
GenerateParam generate = {0, 0, 0, 0, false, PaintWNull, 0};
AxisParam xpar = {0, 0, 0};
AxisParam ypar = {0, 0, 0};
MouseParam mpar = {0, 0, 0, 0, 0, MouseParam::HitNull};
MenuParam menu;
bool open_file_str = false;
TimeGeneration timegen[TimeGenLength];
int timegenpos = 0;

#ifdef __KOLIBRI__

inline int abs(int i) {return (i >= 0) ? i : (-i);}

unsigned int rand_data[4];

void randomize()
{
	rand_data[0] = (unsigned int)Clock();
	rand_data[1] = (unsigned int)GetPackedTime();
	rand_data[2] = (unsigned int)GetPackedDate();
	rand_data[3] = (unsigned int)0xA3901BD2 ^ GetPid();
}

unsigned int rand()
{
	rand_data[0] ^= _HashDword(rand_data[3] + 0x2835C013U);
	rand_data[1] += _HashDword(rand_data[0]);
	rand_data[2] -= _HashDword(rand_data[1]);
	rand_data[3] ^= _HashDword(rand_data[2]);
	return rand_data[3];
}

#define random(k)  (rand() % (k))

#else

#include <stdlib.h>

#endif

/*void DebugPutNumber(int x)
{
	char word[12], *w = word, *s, c;
	int i;
	if (x < 0) {*(w++) = '-'; x = -x;}
	s = w;
	do
	{
		*(s++) = char('0' + (unsigned int)x % 10U);
		(unsigned int&)x /= 10U;
	} while(x);
	for (i = 0; w + i < s - 1 - i; i++)
	{
		c = w[i]; w[i] = s[-1 - i]; s[-1 - i] = c;
	}
	*s = 0;
	DebugPutString(word);
}*/

bool SetPictureSize(int w = -1, int h = -1)
{
	if (w > 32767) w = 32767;
	if (h > 32767) h = 32767;
	if (w > 0) xpar.win = (unsigned short)w;
	if (h > 0) ypar.win = (unsigned short)h;
	if (picture) {Free(picture); picture = 0;}
	if (w == 0 || h == 0 || xpar.win == 0 || ypar.win == 0) return true;
	picture = (unsigned char*)Alloc(3 * xpar.win * ypar.win);
	return picture != 0;

}

bool SetPoleSize(int w = -1, int h = -1)
{
	int s;
	if (w > 32767) w = 32767;
	if (h > 32767) h = 32767;
	if (w > 0) xpar.p = (unsigned short)w;
	if (h > 0) ypar.p = (unsigned short)h;
	if (xpar.p < 4) xpar.p = 4;
	if (ypar.p < 4) ypar.p = 4;
	if (life_data) {Free(life_data); life_data = 0;}
	if (w == 0 || h == 0) return true;
	s = GetDataSize(xpar.p, ypar.p);
	life_data = (unsigned char*)Alloc(2*s + DataMemAdd);
	if (!life_data) return false;
	MemSet(GetDataAddress(life_data), 0, s);
	return true;
}

int GetMenuHeight();

void GetPaintSize(int &w, int &h, int &xx, int &yy)
{
	int t = GetMenuHeight();
	w = xpar.win; h = ypar.win - t;
	xx = 0; yy = t;
}

double GetAxisRatio(const AxisParam &par, int s)
{
	int t = par.p - s;
	if (s <= 0 || t <= 0) return 0;
	return double(t) / 2;
}

void GetAxisShift(const AxisParam &par, int &s, int &k, int &kk)
{
	int t = par.p - s;
	if (t < 0) {kk += (-t) / 2; t = 0; s = par.p;}
	if (s <= 0 || t <= 0) k = 0;
	else
	{
		double r = double(t) / 2;
		k = (int)Floor(r * (1 + par.shift));
		if (k < 0) k = 0;
		else if (k > t) k = t;
	}
}

void GetPaintOrigin(int &w, int &h, int &x, int &y, int &xx, int &yy)
{
	GetPaintSize(w, h, xx, yy);
	GetAxisShift(xpar, w, x, xx);
	GetAxisShift(ypar, h, y, yy);
}

void ApplyScroll(unsigned char *data1 = life_data, unsigned char *data0 = 0)
{
	if (!data0) data0 = data1;
	data0 = (unsigned char*)GetDataAddress(data0);
	data1 = (unsigned char*)GetDataAddress(data1);
	const double min_ratio = 1e-2;
	double r;
	int w, h, xx, yy;
	GetPaintSize(w, h, xx, yy);
	xx = 0; yy = 0;
	r = GetAxisRatio(xpar, w);
	if (menu.button[MenuIHCircle].check)
	{
		xx = mpar.hit_x - mpar.last_x + (int)Floor(xpar.shift * r + 0.5);
		xx %= xpar.p - 2;
		if (xx < 0) xx += xpar.p - 2;
		xpar.shift = 0;
	}
	else if (r < min_ratio) xpar.shift = 0;
	else
	{
		xpar.shift -= double(mpar.last_x - mpar.hit_x) / r;
		if (xpar.shift < -1) xpar.shift = -1;
		else if (xpar.shift > 1) xpar.shift = 1;
	}
	r = GetAxisRatio(ypar, h);
	if (menu.button[MenuIVCircle].check)
	{
		yy = mpar.hit_y - mpar.last_y + (int)Floor(ypar.shift * r + 0.5);
		yy %= ypar.p - 2;
		if (yy < 0) yy += ypar.p - 2;
		ypar.shift = 0;
	}
	else if (r < min_ratio) ypar.shift = 0;
	else
	{
		ypar.shift -= double(mpar.last_y - mpar.hit_y) / r;
		if (ypar.shift < -1) ypar.shift = -1;
		else if (ypar.shift > 1) ypar.shift = 1;
	}
	if (xx == 0 && yy == 0)
	{
		if (data0 != data1) MemCopy(data0, data1, GetDataSize(xpar.p, ypar.p));
	}
	else
	{
		int i, j;
		i = GetDataSize(xpar.p, ypar.p);
		if (data0 == data1)
		{
			data1 += i;
			MemCopy(data1, data0, i);
		}
		MemSet(data0, 0, i);
		APosPixel pixel0(xpar.p, ypar.p, data0);
		APosPixel pixel1(xpar.p, ypar.p, data1);
		for (i = 0; i < xpar.p; i++)
		{
			pixel0.SetTo(i, 0);
			pixel1.SetTo(xx, yy);
			j = ypar.p - yy;
			for (;;)
			{
				if (pixel1.GetPixel()) pixel0.Set1Pixel();
				if (--j == 0) break;
				pixel0.AddY1(); pixel1.AddY1();
			}
			if (yy)
			{
				pixel0.AddY1();
				pixel1.SetTo(xx, 2);
				j = yy;
				for (;;)
				{
					if (pixel1.GetPixel()) pixel0.Set1Pixel();
					if (--j == 0) break;
					pixel0.AddY1(); pixel1.AddY1();
				}
			}
			xx++;
			if (xx >= xpar.p) xx = 2;
		}
	}
}

void DrawLine(int x0, int y0, int x1, int y1, bool c, unsigned char *data0 = life_data)
{
	int i;
	if (y0 == y1)
	{
		if (x0 > x1) {i = x0; x0 = x1; x1 = i;}
		if (x1 < 0 || x0 >= xpar.p || y1 < 0 || y0 >= ypar.p) return;
		if (x0 < 0) x0 = 0;
		if (x1 >= xpar.p) x1 = xpar.p - 1;
		APosPixel pixel(xpar.p, ypar.p, data0, x0, y0);
		for (i = x1 - x0; i >= 0; pixel.AddX1(), i--) pixel.SetPixel(c);
	}
	else if (x0 == x1)
	{
		if (y0 > y1) {i = y0; y0 = y1; y1 = i;}
		if (x1 < 0 || x0 >= xpar.p || y1 < 0 || y0 >= ypar.p) return;
		if (y0 < 0) y0 = 0;
		if (y1 >= ypar.p) y1 = ypar.p - 1;
		APosPixel pixel(xpar.p, ypar.p, data0, x0, y0);
		for (i = y1 - y0; i >= 0; pixel.AddY1(), i--) pixel.SetPixel(c);
	}
	else
	{
		long dx = x1 - x0, dy = y1 - y0;
		int i;
		if (abs(dx) >= abs(dy))
		{
			if (dx < 0)
			{
				i = x0; x0 = x1; x1 = i; dx = -dx;
				y0 = y1; dy = -dy;
			}
			long vy = 0, b_x = dx / 2;
			APosPixel pixel(xpar.p, ypar.p, data0, x0, y0);
			for (i = x0;; i++, pixel.AddX1())
			{
				pixel.SetPixel(c);
				if (i >= x1) break;
				vy += dy;
				if (vy > b_x)
				{
					vy -= dx;
					pixel.AddY1();
				}
				else if (vy < -b_x)
				{
					vy += dx;
					pixel.SubY1();
				}
			}
		}
		else
		{
			if (dy < 0)
			{
				i = y0; y0 = y1; y1 = i; dy = -dy;
				x0 = x1; dx = -dx;
			}
			long vx = 0, b_y = dy / 2;
			APosPixel pixel(xpar.p, ypar.p, data0, x0, y0);
			for (i = y0;; i++, pixel.AddY1())
			{
				pixel.SetPixel(c);
				if (i >= y1) break;
				vx += dx;
				if (vx > b_y)
				{
					vx -= dy;
					pixel.AddX1();
				}
				else if (vx < -b_y)
				{
					vx += dy;
					pixel.SubX1();
				}
			}
		}
	}
}

void FillCircle(int x0, int y0, int r, bool c, unsigned char *data0 = life_data)
{
	int x = 0, y = r, v = 0;
	while (x <= y)
	{
		DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, c, data0);
		if (y) DrawLine(x0 - x, y0 - y, x0 + x, y0 - y, c, data0);
		if (x < y)
		{
			DrawLine(x0 - y, y0 + x, x0 + y, y0 + x, c, data0);
			DrawLine(x0 - y, y0 - x, x0 + y, y0 - x, c, data0);
		}
		v += 2 * (x++) + 1;
		if (v >= y) v -= 2 * (y--) - 1;
	}
}

void RandomDraw(unsigned char *data0 = life_data)
{
	if (!data0 || random(300) >= 1) return;
	int d = xpar.p;
	if (d > ypar.p) d = ypar.p;
	data0 = (unsigned char*)GetDataAddress(data0);
	d = random((d * 3) / 4);
	int x = random(xpar.p - d), y = random(ypar.p - d);
	if (random(10) < 1)
	{
		int NBusy, NTest = 4096;
		NBusy = 5 * xpar.p * ypar.p;
		if (NTest > NBusy) NTest = NBusy;
		NBusy = 0;
		for (int k = 0; k < NTest; k++)
		{
			if (GetDataBit(GetDataWidth(xpar.p), GetDataHeight(ypar.p), data0, random(xpar.p), random(ypar.p))) NBusy++;
		}
		if (NBusy * 100 < NTest)
		{
			if (random(3) == 0)
			{
				DrawLine(x, y, x + d, y + d, true, data0);
				DrawLine(x, y + d, x + d, y, true, data0);
			}
			else
			{
				DrawLine(x + d/2, y, x + d/2, y + d, true, data0);
				DrawLine(x, y + d/2, x + d, y + d/2, true, data0);
			}
			return;
		}
	}
	if (2*d < xpar.p && 2*d < ypar.p && random(10) < 3)
	{
		FillCircle(x + d/2, y + d/2, d/2, false, data0);
	}
	else if (random(2)) DrawLine(x, y, x + d, y + d, true, data0);
	else DrawLine(x, y + d, x + d, y, true, data0);
}

void LineInScreen(int ax, int ay, int bx, int by, bool c, unsigned char *data0 = life_data)
{
	int mul = 1, sbeg, send, t0, t1;
	if (ax != bx) mul *= abs(ax - bx);
	if (ay != by) mul *= abs(ay - by);
	sbeg = 0; send = mul;
	if (ax != bx)
	{
		t0 = ax * (mul / (ax - bx));
		t1 = (ax - xpar.p + 1) * (mul / (ax - bx));
		if (t0 < t1)
		{
			if (sbeg < t0) sbeg = t0;
			if (send > t1) send = t1;
		}
		else
		{
			if (sbeg < t1) sbeg = t1;
			if (send > t0) send = t0;
		}
	}
	else if (ax < 0 || ax >= xpar.p) return;
	if (ay != by)
	{
		t0 = ay * (mul / (ay - by));
		t1 = (ay - ypar.p + 1) * (mul / (ay - by));
		if (t0 < t1)
		{
			if (sbeg < t0) sbeg = t0;
			if (send > t1) send = t1;
		}
		else
		{
			if (sbeg < t1) sbeg = t1;
			if (send > t0) send = t0;
		}
	}
	else if (ay < 0 || ay > ypar.p) return;
	if (send < sbeg) return;
	DrawLine(ax + (bx - ax) * sbeg / mul, ay + (by - ay) * sbeg / mul,
			ax + (bx - ax) * send / mul, ay + (by - ay) * send / mul, c, data0);
}

int GetRadius(int ax, int ay, int bx, int by)
{
	int s, t0, t1, t, tt;
	bx -= ax; by -= ay;
	s = bx*bx + by*by;
	t0 = 0; t1 = s;
	while (t0 + 1 < t1)
	{
		t = (t0 + t1) / 2;
		tt = t*t;
		if (tt / t == t && s > tt + t) t0 = t; else t1 = t;
	}
	return t1;
}

int ReadNumberFromString(const unsigned char *&str)
{
	int x = 0, s = 1;
	while (*str == ' ' || *str == '\t' || *str == '\r') str++;
	if (*str == '-') {s = -1; str++;}
	else if (*str == '+') str++;
	while (*str >= '0' && *str <= '9')
	{
		x = 10*x + (*str - '0');
		str++;
	}
	return x*s;
}

const unsigned char *StringPrefSpace(const unsigned char *pict, int size, const unsigned char *pref)
{
	const unsigned char *pict_end = pict + size;
	for (;;)
	{
		if (!*pref) return pict;
		else if (*pref == ' ')
		{
			if (pict >= pict_end || !(*pict == ' ' || *pict == '\t' || *pict == '\r')) return 0;
			while (pict < pict_end && (*pict == ' ' || *pict == '\t' || *pict == '\r')) pict++;
			pref++;
		}
		else if (*pref == '\n')
		{
			while (pict < pict_end && (*pict == ' ' || *pict == '\t' || *pict == '\r')) pict++;
			if (pict >= pict_end || *pict != '\n') return 0;
			pict++; pref++;
		}
		else if (pict >= pict_end || *pict != *pref) return 0;
		else {pict++; pref++;}
	}
}

int LifeGetPictureType(const unsigned char *&pict, int size)
{
	const unsigned char *p;
	p = StringPrefSpace(pict, size, (const unsigned char*)"#LifeBin 2.0\n");
	if (p && p + 4 <= pict + size) {pict = p; return 1;}
	p = StringPrefSpace(pict, size, (const unsigned char*)"#Life 1.05\n");
	if (p) {pict = p; return 2;}
	if (size >= 54 && pict[0] == 'B' && pict[1] == 'M' && *(int*)(pict+6) == 0 &&
		*(int*)(pict+14) == 0x28 && *(short*)(pict+26) == 1 && *(int*)(pict+30) == 0 &&
		*(short*)(pict+28) > 0 && *(short*)(pict+28) <= 32 &&
		*(int*)(pict+18) >= 0 && *(int*)(pict+22) >= 0 &&
		*(int*)(pict+10) >= 54 && *(int*)(pict+10) <= *(int*)(pict+2) &&
		*(int*)(pict+2) <= size && *(int*)(pict+34) >= 0 &&
		*(int*)(pict+34) <= *(int*)(pict+2) - *(int*)(pict+10) &&
		*(int*)(pict+46) <= 256 && *(int*)(pict+50) <= *(int*)(pict+46) &&
		(*(short*)(pict+28) >= 8 || *(int*)(pict+46) <= (1 << *(short*)(pict+28))))
	{
		if (*(int*)(pict+18) == 0 || *(int*)(pict+22) == 0) return 3;
		int s = *(int*)(pict+34);
		if (s == 0)
		{
			s = ((*(int*)(pict+18) * *(short*)(pict+28) - 1) / 32 + 1) * *(int*)(pict+22) * 4;
		}
		if (s > 0 && s <= *(int*)(pict+2) - *(int*)(pict+10))
		{
			s /= *(int*)(pict+22);
			if (s < (1 << 28) && (s * 8) / *(short*)(pict+28) >= *(int*)(pict+18)) return 3;
		}
	}
	return 0;
}

void LifeGetPictureSize(int &w, int &h, const unsigned char *pict, int size)
{
	const unsigned char *pict_end = pict + size;
	int type = LifeGetPictureType(pict, size);
	w = 0; h = 0;
	if (type == 1)
	{
		w = (int)pict[0] + ((int)pict[1] << 8);
		h = (int)pict[2] + ((int)pict[3] << 8);
	}
	else if (type == 2)
	{
		int x = 0, y = 0, xb = x;
		int x0 = 0, y0 = 0, x1 = -1, y1 = -1;
		while (pict < pict_end && *pict)
		{
			while (pict < pict_end && *pict == '\n') pict++;
			if (pict < pict_end && *pict == '#')
			{
				pict++;
				if (pict < pict_end && (*pict == 'p' || *pict == 'P'))
				{
					pict++;
					x = ReadNumberFromString(pict);
					y = ReadNumberFromString(pict);
					xb = x;
				}
				while (pict < pict_end && *pict)
				{
					if (*pict == '\n') {pict++; break;}
					pict++;
				}
				continue;
			}
			for (; pict < pict_end && *pict; pict++)
			{
				if (*pict == '\n')
				{
					x = xb; y++;
					if (pict + 1 < pict_end && pict[1] == '#') break;
					continue;
				}
				else if (*pict == '\r') continue;
				if (*pict == '*')
				{
					if (x0 > x) x0 = x;
					if (x1 < x) x1 = x;
					if (y0 > y) y0 = y;
					if (y1 < y) y1 = y;
				}
				x++;
			}
		}
		x0 = -2*x0; x1 = 2*x1 + 1;
		y0 = -2*y0; y1 = 2*y1 + 1;
		w = (x0 < x1) ? x1 : x0;
		h = (y0 < y1) ? y1 : y0;
	}
	else if (type == 3)
	{
		w = *(int*)(pict+18);
		h = *(int*)(pict+22);
		if (w == 0) h = 0;
		else if (h == 0) w = 0;
	}
}

void LifePutPicture(int x0, int y0, const unsigned char *pict, int size, unsigned char *data0 = life_data)
{
	const unsigned char *pict_end = pict + size;
	int type = LifeGetPictureType(pict, size);
	if (type == 1)
	{
		int w = (int)pict[0] + ((int)pict[1] << 8);
		int h = (int)pict[2] + ((int)pict[3] << 8);
		if (w && h)
		{
			int i, j, x, y;
			pict += 4;
			x0 -= w / 2; y0 -= h / 2;
			x = x0 + w; y = y0 - 1;
			APosPixel pixel(xpar.p, ypar.p, data0);
			while (pict < pict_end)
			{
				if (x >= x0 + w)
				{
					i = (x - x0) / w;
					x -= i * w; y += i;
					if (y >= y0 + h) break;
					j = 0;
					if (x >= 0 && x < xpar.p) j |= 1;
					if (y >= 0 && y < ypar.p) j |= 2;
					if (j == 3)	pixel.SetTo(x, y);
				}
				i = *(pict++);
				if (i == 0)
				{
					if (j == 3) pixel.Set1Pixel();
					i = 1;
				}
				x += i;
				if ((j & 2) && x < x0 + w)
				{
					if (x >= 0 && x < xpar.p)
					{
						if ((j & 1) && i < 5)
						{
							while (i--) pixel.AddX1();
						}
						else
						{
							j |= 1;
							pixel.SetTo(x, y);
						}
					}
					else j &= ~1;
				}
			}
		}
	}
	else if (type == 2)
	{
		int x = x0, y = y0, xb = x;
		while (pict < pict_end && *pict)
		{
			while (pict < pict_end && *pict == '\n') pict++;
			if (pict < pict_end && *pict == '#')
			{
				pict++;
				if (pict < pict_end && (*pict == 'p' || *pict == 'P'))
				{
					pict++;
					x = x0 + ReadNumberFromString(pict);
					y = y0 + ReadNumberFromString(pict);
					xb = x;
				}
				while (pict < pict_end && *pict)
				{
					if (*pict == '\n') {pict++; break;}
					pict++;
				}
				continue;
			}
			if (y >= ypar.p || x >= xpar.p)
			{
				for (; pict < pict_end && *pict; pict++) if (*pict == '\n')
				{
					y++;
					if (pict + 1 < pict_end && pict[1] == '#') break;
				}
				continue;
			}
			if (y < 0)
			{
				for (; pict < pict_end && *pict; pict++) if (*pict == '\n')
				{
					y++;
					if (y >= 0 || (pict + 1 < pict_end && pict[1] == '#')) break;
				}
				if (pict + 1 < pict_end && *pict == '\n' && pict[1] == '#') continue;
			}
			APosPixel pixel(xpar.p, ypar.p, data0);
			if (x >= 0) pixel.SetTo(x, y);
			for (; pict < pict_end && *pict; pict++)
			{
				if (*pict == '\n')
				{
					x = xb; y++;
					if (y >= ypar.p) break;
					if (x >= 0) pixel.SetTo(x, y);
					if (pict + 1 < pict_end && pict[1] == '#') break;
					continue;
				}
				else if (*pict == '\r') continue;
				if (*pict == '*') pixel.Set1Pixel();
				x++;
				if (x < 0) continue;
				if (x >= xpar.p)
				{
					while (pict < pict_end && *pict && *pict != '\n') pict++;
					if (pict < pict_end && *pict == '\n') pict--;
					continue;
				}
				if (x == 0) pixel.SetTo(0, y);
				else pixel.AddX1();
			}
		}
	}
	else if (type == 3)
	{
		int w = *(int*)(pict+18), h = *(int*)(pict+22);
		if (w && h)
		{
			int n, i, j;
			unsigned char ch;
			const unsigned char *p = pict + *(int*)(pict+10);
			short bp = *(short*)(pict+28);
			int s = *(int*)(pict+34);
			x0 -= w / 2; y0 -= h / 2;
			if (x0 < xpar.p && y0 < ypar.p && x0 + w > 0 && y0 + h > 0)
			{
				if (s) s /= *(int*)(pict+22);
				else s = ((*(int*)(pict+18) * *(short*)(pict+28) - 1) / 32 + 1) * 4;
				n = (*(int*)(pict+10) - 54) / 4;
				APosPixel pixel(xpar.p, ypar.p, data0);
				if (y0 + h <= ypar.p) i = h - 1;
				else
				{
					i = ypar.p - y0 - 1;
					p += (ypar.p - y0) * s;
				}
				for (; i >= 0; i--)
				{
					int tj = 0, tl = 0;
					if (y0 + i < 0) break;
					if (x0 > 0) pixel.SetTo(x0, y0 + i);
					for (j = 0; j < 8*s; j += 8)
					{
						if (tj >= w || x0 + tj >= xpar.p) {p += (s - j/8); break;}
						ch = *(p++);
						while (tj < w && x0 + tj < xpar.p && j + 8 >= (tj+1) * bp)
						{
							union
							{
								long za;
								unsigned char z[4];
							};

							tl |= (unsigned long)(ch) >> ((int)j + 8 - (tj+1) * bp);
							if (n)
							{
								if (tl >= n) za = 0;
								else
								{
									const unsigned char *zp = pict + 54 + 4*tl;
									z[0] = zp[3];
									z[1] = zp[2];
									z[2] = zp[1];
									z[3] = zp[0];
								}
							}
							else if (bp == 8)
							{
								z[0] = 0;
								z[1] = z[2] = z[3] = (char)tl;
							}
							else if (bp == 32) za = tl;
							else za = tl << (32 - bp);

							if (x0 + tj >= 0)
							{
								if (x0 + tj == 0) pixel.SetTo(0, y0 + i);
								else pixel.AddX1();
								if ((int)z[1] + (int)z[2] + (int)z[3] >= 384)
								{
									pixel.Set1Pixel();
								}
							}

							tl = 0;
							ch &= (unsigned char)((1 << ((int)j + 8 - (tj+1) * bp)) - 1);
							tj++;
						}
						tl |= (int)ch << ((tj+1) * bp - (j + 8));
					}
				}
			}
		}
	}
}

void ApplyHit(unsigned char *data1 = life_data, unsigned char *data0 = 0)
{
	if (!data0) data0 = data1;
	if (!data0) return;
	data0 = (unsigned char*)GetDataAddress(data0);
	data1 = (unsigned char*)GetDataAddress(data1);
	if (data0 != data1 && mpar.hit_type != MouseParam::HitScroll)
	{
		MemCopy(data0, data1, GetDataSize(xpar.p, ypar.p));
	}
	switch (mpar.hit_type)
	{
	case MouseParam::HitLine:
		LineInScreen(mpar.hit_x, mpar.hit_y, mpar.last_x, mpar.last_y, true, data0);
		break;
	case MouseParam::HitCircle:
		FillCircle(mpar.hit_x, mpar.hit_y,
				GetRadius(mpar.hit_x, mpar.hit_y, mpar.last_x, mpar.last_y), false, data0);
		break;
	case MouseParam::HitScroll:
		ApplyScroll(data1, data0);
		break;
	}
}

void MoveGenerateTime(unsigned int t)
{
	static const unsigned int COUNT_MAX = 1 << 24;

	if (generate.stop)
	{
		if (generate.count > COUNT_MAX) generate.count = COUNT_MAX;
	}
	else if (!generate.gps) generate.count = COUNT_MAX;
	else if (t > 100 || generate.count >= generate.gps)
	{
		generate.count = generate.gps;
	}
	else if (t)
	{
		generate.count += (generate.gps * t -
					(((generate.time + t) % 100U) * generate.gps) % 100 +
					((generate.time % 100U) * generate.gps) % 100) / 100;
		if (generate.count > generate.gps) generate.count = generate.gps;
	}
	generate.time += t;
	if (timegen[timegenpos].t > (~t)) timegen[timegenpos].t = -1;
	else timegen[timegenpos].t += t;
}

void ResetGenerate()
{
	generate.time = Clock();
	generate.paint_time = generate.time - 100;
	generate.count = 0;
	if (generate.stop)
	{
		generate.stop = false;
		menu.button[MenuIGenerate].check = false;
	}
}

void InitGenerate()
{
	int i;
	for (i = 0; i < TimeGenLength; i++)
	{
		timegen[timegenpos].t = -1;
		timegen[timegenpos].g = 0;
	}
	ResetGenerate();
}

bool AddGenerateCount(int c)
{
	if (c < 0) return false;
	if (!menu.button[MenuIGenerate].check)
	{
		ResetGenerate();
		menu.button[MenuIGenerate].check = true;
		generate.paint |= PaintWMenuBorder | PaintWSpeed;
		generate.stop = true;
		generate.count += c;
	}
	else if (generate.stop) generate.count += c;
	else return false;
	return true;
}

void InitMenuButton()
{
	int i;
	const unsigned char *p = menu_picture, *p_end = menu_picture + sizeof(menu_picture);
	const unsigned int separator = 5;
	for (i = 0; i < MenuParam::NButton; i++)
	{
		menu.button[i].left = 0;
		menu.button[i].size = MenuParam::Size - 2;
		menu.button[i].border = 2;
		menu.button[i].check = false;
		menu.button[i].bitmap = p;
		if (p && !p[0])
		{
			menu.button[i].bitmap = 0;
			if (p[1]) p = 0;
		}
		if (p)
		{
			p += 2 + 3 * (int)p[0] * (int)p[1];
			if (p > p_end) menu.button[i].bitmap = 0;
			if (p >= p_end) p = 0;
		}
	}
	p = digits_picture; p_end = digits_picture + sizeof(digits_picture);
	for (i = 0; i < MenuDig; i++)
	{
		menu.digit[i] = p;
		if (p && !p[0])
		{
			menu.digit[i] = 0;
			if (p[1]) p = 0;
		}
		if (p)
		{
			p += 2 + ((int)p[0] * (int)p[1] + 7) / 8;
			if (p > p_end) menu.digit[i] = 0;
			if (p >= p_end) p = 0;
		}
	}
	menu.draw = false;
	menu.pressed = -1;
	menu.current = MenuILine;
	menu.edit = -1;
	menu.button[menu.current].check = true;
	menu.button[MenuIHide].size /= 2;
	menu.button[MenuIGenerate].left += separator;
	menu.button[MenuIGenerate].check = true;
	menu.button[MenuIRandom].check = true;
	menu.button[MenuILine].left += separator;
	menu.button[MenuIWinSize].left += separator;
	menu.button[MenuISize].size += 80;
	menu.button[MenuISpeed].left += separator;
	menu.button[MenuISpeed].size += 60;
	menu.button[0].left = 1;
	for (i = 1; i < MenuParam::NButton; i++)
	{
		menu.button[i].left += menu.button[i-1].left + menu.button[i-1].size;
	}
}

int GetMenuYPos()
{
	return 0;
}

int GetMenuHeight()
{
	if (!menu.draw) return 0;
	return (ypar.win <= MenuParam::Size) ? 0 : MenuParam::Size;
}

int GetMenuNumber(int k, int i)
{
	if (k == menu.edit) return menu.edit_num[i];
	switch (k)
	{
	case MenuISize:
		return i ? ypar.p : xpar.p;
	case MenuISpeed:
		return menu.button[MenuIGenerate].check ? (int)Floor(generate.speed + 0.5) : 0;
	default:
		return 0;
	}
}

void WinDrawRect(int x, int y, int w, int h, const unsigned char *const *color)
{
	unsigned char *p = picture + 3 * (y * xpar.win + x);
	int j;
	w--; h--;
	for (j = w; j > 0; j--)
	{
		p[0] = color[0][0]; p[1] = color[0][1]; p[2] = color[0][2];
		p += 3;
	}
	for (j = h; j > 0; j--)
	{
		p[0] = color[1][0]; p[1] = color[1][1]; p[2] = color[1][2];
		p += 3 * xpar.win;
	}
	for (j = w; j > 0; j--)
	{
		p[0] = color[2][0]; p[1] = color[2][1]; p[2] = color[2][2];
		p -= 3;
	}
	for (j = h; j > 0; j--)
	{
		p[0] = color[3][0]; p[1] = color[3][1]; p[2] = color[3][2];
		p -= 3 * xpar.win;
	}
}

void WinFillRect(int x, int y, int w, int h, const unsigned char *color)
{
	if (x >= xpar.win || y >= ypar.win || w <= 0 || h <= 0) return;
	if (w > xpar.win - x) w = xpar.win - x;
	if (h > ypar.win - y) h = ypar.win - y;
	unsigned char *p, *pp = picture + 3 * (y * xpar.win + x);
	int i, j;
	for (i = h; i > 0; i--)
	{
		p = pp;
		for (j = w; j > 0; j--)
		{
			*(p++) = color[0]; *(p++) = color[1]; *(p++) = color[2];
		}
		pp += 3 * xpar.win;
	}
}

void WinBitmapRect(int x, int y, const unsigned char *bmp)
{
	if (!bmp || !bmp[0] || !bmp[1]) return;
	int w = bmp[0], h = bmp[1], strl = 3 * (int)bmp[0];
	bmp += 2;
	x -= w/2; y -= h/2;
	if (x >= xpar.win || y >= ypar.win) return;
	if (w > xpar.win - x) w = xpar.win - x;
	if (h > ypar.win - y) h = ypar.win - y;
	unsigned char *p, *pp = picture + 3 * (y * xpar.win + x);
	const unsigned char *b;
	int i, j;
	for (i = h; i > 0; i--)
	{
		p = pp; b = bmp;
		for (j = w; j > 0; j--)
		{
			*(p++) = *(b++); *(p++) = *(b++); *(p++) = *(b++);
		}
		pp += 3 * xpar.win; bmp += strl;
	}
}

void WinBitSetRect(int x, int y, const unsigned char *set, const unsigned char *color)
{
	if (!set || !set[0] || !set[1]) return;
	int w = set[0], h = set[1], strr = (int)set[0];
	set += 2;
	x -= w/2; y -= h/2;
	if (x >= xpar.win || y >= ypar.win) return;
	if (w > xpar.win - x) w = xpar.win - x;
	if (h > ypar.win - y) h = ypar.win - y;
	strr -= w;
	unsigned char *p, *pp = picture + 3 * (y * xpar.win + x);
	int i, j, m = 1;
	for (i = h; i > 0; i--)
	{
		p = pp;
		for (j = w; j > 0; j--)
		{
			if (*set & m) {p[0] = color[0]; p[1] = color[1]; p[2] = color[2];}
			p += 3;
			m <<= 1;
			if (!(m & 255)) {m = 1; set++;}
		}
		pp += 3 * xpar.win;
		m <<= strr % 8; set += strr / 8;
		if (!(m & 255)) {m >>= 8; set++;}
	}
}

void WinNumberRect(int x, int y, unsigned int n, const unsigned char *color)
{
	int w, m, i;
	w = 0; m = n;
	do
	{
		i = m % MenuDig; m /= MenuDig;
		if (menu.digit[i]) w += 2 + menu.digit[i][0];
	} while(m);
	if (w <= 2) return;
	x += w - (w-2) / 2;
	m = n;
	do
	{
		i = m % MenuDig; m /= MenuDig;
		if (menu.digit[i])
		{
			x -= 2 + menu.digit[i][0];
			WinBitSetRect(x + menu.digit[i][0] / 2, y, menu.digit[i], color);
		}
	} while(m);
}

void WinNumberEditRect(int x, int y, int w, int h, unsigned int n,
			const unsigned char *color, const unsigned char *bg_color)
{
	if (bg_color) WinFillRect(x, y, w, h, bg_color);
	WinNumberRect(x + w/2, y + h/2, n, color);
}

void MenuPaint(int what)
{
	static const unsigned char color_light0[3] = {255, 255, 255};
	static const unsigned char color_light[3] = {208, 208, 208};
	static const unsigned char color_face[3] = {192, 192, 192};
	static const unsigned char color_shadow[3] = {128, 128, 128};
	static const unsigned char color_shadow0[3] = {64, 64, 64};
	static const unsigned char color_black[3] = {0, 0, 0};
	static const unsigned char (&color_white)[3] = color_light0;

	if (GetMenuHeight() < MenuParam::Size) return;
	const unsigned char *color[4];
	int menuy = GetMenuYPos(), i, k, x, xx, y, yy;
	if ((what & PaintWSpeed) && !(what & (PaintWMenu & ~PaintWMenuBorder)) &&
				menu.button[MenuISpeed].Right() < xpar.win)
	{
		k = MenuISpeed;
		i = menu.button[k].border + 1;
		xx = menu.button[k].Left() + i;
		yy = menuy + 1 + i;
		x = menu.button[k].size - 2*i;
		y = MenuParam::Size - 2 - 2*i;
		i = 0;
		if (menu.button[k].bitmap) i += menu.button[k].bitmap[0] + 2;
		WinNumberEditRect(xx + i, yy, x - i, y, GetMenuNumber(k, 0), color_black,
					(menu.edit == k && menu.edit_index == 0) ? color_white : color_face);
	}
	if (!(what & PaintWMenu)) return;
	if (what & (PaintWMenu & ~PaintWMenuBorder))
	{
		x = menu.button[MenuParam::NButton - 1].Right();
		WinFillRect(0, menuy, x, 1, color_face);
		WinFillRect(0, menuy + MenuParam::Size - 1, x, 1, color_face);
		WinFillRect(x, menuy, xpar.win - x, MenuParam::Size, color_face);
	}
	for (k = 0; k < MenuParam::NButton; k++)
	{
		xx = menu.button[k].Left();
		yy = menuy + 1;
		x = menu.button[k].size;
		y = MenuParam::Size - 2;
		if (xx + x >= xpar.win)
		{
			if (what & (PaintWMenu & ~PaintWMenuBorder))
			{
				i = (k >= 1) ? menu.button[k-1].Right() : 0;
				WinFillRect(i, yy, xpar.win - i, y, color_face);
			}
			break;
		}
		if (what & (PaintWMenu & ~PaintWMenuBorder))
		{
			i = (k >= 1) ? menu.button[k-1].Right() : 0;
			WinFillRect(i, yy, xx - i, y, color_face);
		}
		for (i = 0; i < menu.button[k].border; i++)
		{
			if (i <= 1)
			{
				if (menu.button[k].check)
				{
					color[0] = color[3] = i ? color_shadow : color_shadow0;
					color[1] = color[2] = i ? color_light : color_light0;
				}
				else
				{
					color[0] = color[3] = i ? color_light : color_light0;
					color[1] = color[2] = i ? color_shadow : color_shadow0;
				}
			}
			WinDrawRect(xx, yy, x, y, color);
			xx++; yy++; x -= 2; y -= 2;
		}
		if (what & (PaintWMenu & ~PaintWMenuBorder))
		{
			WinFillRect(xx, yy, x, y, color_face);
			if (menu.button[k].bitmap)
			{
				i = (k == MenuISpeed) ? (1 + menu.button[k].bitmap[0] / 2) : (x / 2);
				WinBitmapRect(xx + i, yy + y/2, menu.button[k].bitmap);
			}
			if (k == MenuISize)
			{
				xx++; yy++; x -= 2; y -= 2;
				i = x - 4;
				if (menu.button[k].bitmap) i -= menu.button[k].bitmap[0];
				i /= 2;
				WinNumberEditRect(xx, yy, i, y, GetMenuNumber(k, 0), color_black,
							(menu.edit == k && menu.edit_index == 0) ? color_white : 0);
				WinNumberEditRect(xx + x - i, yy, i, y, GetMenuNumber(k, 1), color_black,
							(menu.edit == k && menu.edit_index == 1) ? color_white : 0);
			}
			else if (k == MenuISpeed)
			{
				xx++; yy++; x -= 2; y -= 2;
				i = 0;
				if (menu.button[k].bitmap) i += menu.button[k].bitmap[0] + 2;
				WinNumberEditRect(xx + i, yy, x - i, y, GetMenuNumber(k, 0), color_black,
							(menu.edit == k && menu.edit_index == 0) ? color_white : 0);
			}
		}
	}
}

void Paint(int what, TThreadData th);

void SetMenuDraw(bool draw, TThreadData th)
{
	if (draw == menu.draw) return;
	if (menu.pressed >= 0) menu.button[menu.pressed].check = false;
	menu.pressed = -1;
	menu.draw = draw;
	Paint(PaintWAll | PaintWFast, th);
}

void SetMenuPressed(int k, TThreadData th)
{
	if (menu.pressed == k) return;
	if (menu.pressed >= 0) menu.button[menu.pressed].check = false;
	if (k >= 0) menu.button[k].check = true;
	menu.pressed = k;
	Paint(PaintWMenuBorder | PaintWFast, th);
}

void SetMenuCurrent(int k, TThreadData th)
{
	if (menu.current == k) return;
	if (menu.current >= 0) menu.button[menu.current].check = false;
	if (k >= 0) menu.button[k].check = true;
	menu.current = k;
	Paint(PaintWMenuBorder | PaintWFast, th);
}

void SetMenuEdit(int k, int i, TThreadData th)
{
	if (menu.edit != k)
	{
		if (menu.edit >= 0) menu.button[menu.edit].check = false;
		if (k >= 0) menu.button[k].check = true;
		if (k == MenuISize) {menu.edit_num[0] = xpar.p; menu.edit_num[1] = ypar.p;}
		else if (k == MenuISpeed) menu.edit_num[0] = generate.gps;
	}
	else if (menu.edit_index == i) return;
	if (k == MenuISize) menu.edit_num_max = 32767;
	else if (k == MenuISpeed) menu.edit_num_max = 9999999;
	menu.edit = k; menu.edit_index = i;
	Paint(PaintWMenu | PaintWFast, th);
}

void ApplyMenuEdit(TThreadData th)
{
	if (menu.edit < 0) return;
	if (menu.edit == MenuISize)
	{
		int w = menu.edit_num[0], h = menu.edit_num[1];
		ResetGenerate();
		if (xpar.p != w || ypar.p != h)
		{
			if (w <= 0) w = 1;
			if (h <= 0) h = 1;
			SetPoleSize(w, h);
			generate.paint |= PaintWPole | PaintWMenu | PaintWFast;
		}
	}
	else if (menu.edit == MenuISpeed)
	{
		generate.gps = menu.edit_num[0];
	}
	SetMenuEdit(-1, -1, th);
}

int GetMenuEditIndex(int k, int x)
{
	if (k == MenuISize) return x >= menu.button[k].left + menu.button[k].size / 2;
	else return 0;
}

void LifeScreenPutPicture(const unsigned char *pict, int size, TThreadData th)
{
	int w, h;
	ResetGenerate();
	LifeGetPictureSize(w, h, pict, size);
	w += 10; h += 10;
	if (!life_data || xpar.p < w || ypar.p < h)
	{
		if (xpar.p >= w) w = xpar.p;
		if (ypar.p >= h) h = ypar.p;
		SetPoleSize(w, h);
		if (!life_data)
		{
			Paint(PaintWMenu | PaintWFast, th);
			return;
		}
	}
	MemSet(GetDataAddress(life_data), 0, GetDataSize(xpar.p, ypar.p));
	LifePutPicture(xpar.p / 2, ypar.p / 2, pict, size, life_data);
	menu.button[MenuIRandom].check = false;
	xpar.shift = 0; ypar.shift = 0;
	generate.paint |= PaintWPole | PaintWMenu | PaintWFast;
//	SetMenuCurrent(MenuIScroll, th);
	Paint(PaintWNull, th);
}

void MenuOpenDialogEnd(TThreadData th)
{
	if(!ofd.openfile_path[0] || !open_file_str) return;
	open_file_str = false;
	char *name = ofd.openfile_path;
	if (!name) return;
	FileInfoBlock* file = FileOpen(name);
	if (!file) return;
	int k = FileGetLength(file);
	unsigned char *pict = 0;
	if (k > 0 && k < (1 << 24))
	{
		pict = (unsigned char*)Alloc(k+1);
		if (pict)
		{
			if (FileRead(file, pict, k) == k) pict[k] = 0;
			else {Free(pict); pict = 0;}
		}
	}
	FileClose(file);
	if (!pict) return;
	LifeScreenPutPicture(pict, k, th);
	Free(pict);
}

void MenuWinSizeClick(TThreadData th)
{
	int w = xpar.win, h = ypar.win - GetMenuHeight();
	ResetGenerate();
	if (w > 0 && h > 0 && (xpar.p != w || ypar.p != h))
	{
		SetPoleSize(w, h);
		Paint(PaintWPole | PaintWMenu | PaintWFast, th);
	}
}

void MenuGenerateClick(TThreadData th)
{
	generate.stop = false;
	ResetGenerate();
	menu.button[MenuIGenerate].check = !menu.button[MenuIGenerate].check;
	Paint(PaintWMenuBorder | PaintWSpeed | PaintWFast, th);
}

void MenuClearClick(TThreadData th)
{
	ResetGenerate();
	if (life_data) MemSet(GetDataAddress(life_data), 0, GetDataSize(xpar.p, ypar.p));
	Paint(PaintWPole | PaintWFast, th);
}

void MenuAboutClick(TThreadData th)
{
	generate.stop = false;
	menu.button[MenuIGenerate].check = true;
	generate.paint |= PaintWSpeed;
	LifeScreenPutPicture(about_picture, sizeof(about_picture), th);
}

void MenuMouseClick(int x, int y, int m, TThreadData th)
{
	int k, i, j = GetMenuYPos();
	if (menu.edit >= 0)
	{
		k = menu.edit;
		j = GetMenuYPos();
		if (GetMenuHeight() < MenuParam::Size || y < j + 1 || y >= j + MenuParam::Size - 1 ||
					x < menu.button[k].Left() || x >= menu.button[k].Right())
		{
			if (m == 1) ApplyMenuEdit(th);
			else SetMenuEdit(-1, -1, th);
		}
		else SetMenuEdit(k, GetMenuEditIndex(k, x), th);
		return;
	}
	if (GetMenuHeight() < MenuParam::Size || y < j + 1 || y >= j + MenuParam::Size - 1)
	{
		if (m < 0) x = -1;
		else return;
	}
	if (m < 0)
	{
		if (menu.pressed < 0) return;
		k = menu.pressed;
		if (x < menu.button[k].Left() || x >= menu.button[k].Right())
		{
			if (menu.button[k].check)
			{
				menu.button[k].check = false;
				Paint(PaintWMenuBorder | PaintWFast, th);
			}
		}
		else if (!menu.button[k].check)
		{
			menu.button[k].check = true;
			Paint(PaintWMenuBorder | PaintWFast, th);
		}
		return;
	}
	if (m == 0)
	{
		if (menu.pressed < 0 || !menu.button[menu.pressed].check) return;
		switch (menu.pressed)
		{
		case MenuIHide:
			SetMenuDraw(false, th);
			break;
		case MenuIClear:
			MenuClearClick(th);
			break;
		case MenuIOpen:
			ofd.type = 0; // 0 - open
			OpenDialog_Start(&ofd);
			if(ofd.status==1) open_file_str = true;
			break;
		case MenuIAbout:
			MenuAboutClick(th);
			break;
		case MenuIExit:
			CloseWindow(th);
			break;
		case MenuIWinSize:
			MenuWinSizeClick(th);
			break;
		}
		return;
	}
	k = -1; i = MenuParam::NButton;
	while (k + 1 < i)
	{
		j = (k + i) / 2;
		if (x < menu.button[j].Left()) i = j;
		else k = j;
	}
	i = menu.button[k].Right();
	if (k < 0 || x >= i || i >= xpar.win) return;
	switch (k)
	{
	case MenuIHide:
	case MenuIClear:
	case MenuIOpen:
	case MenuIAbout:
	case MenuIExit:
	case MenuIWinSize:
		SetMenuPressed(k, th);
		break;
	case MenuIGenerate:
		MenuGenerateClick(th);
		break;
	case MenuIRandom:
	case MenuIVCircle:
	case MenuIHCircle:
		menu.button[k].check = !menu.button[k].check;
		Paint(PaintWMenuBorder | PaintWFast, th);
		break;
	case MenuILine:
	case MenuIScroll:
		SetMenuCurrent(k, th);
		break;
	case MenuISize:
	case MenuISpeed:
		SetMenuEdit(k, GetMenuEditIndex(k, x), th);
		break;
	}
}

void PoleMouseClick(int m, TThreadData th)
{
	if (m != 1 && m != 2) return;
	mpar.hit_type = MouseParam::HitNull;
	switch (menu.current)
	{
	case MenuILine:
		if (menu.draw)
		{
			menu.button[MenuIRandom].check = false;
			generate.paint |= PaintWMenuBorder | PaintWFast;
			if (m == 1) mpar.hit_type = MouseParam::HitLine;
			else mpar.hit_type = MouseParam::HitCircle;
		}
		break;
	case MenuIScroll:
		if (m == 1) mpar.hit_type = MouseParam::HitScroll;
		break;
	}
	if (mpar.hit_type) Paint(PaintWPole | PaintWFast, th);
	else if (!menu.draw) SetMenuDraw(true, th);
}

void MenuEditPressKey(int ch, TThreadData th)
{
	if (menu.edit < 0) return;
	int &num = menu.edit_num[menu.edit_index];
	if (ch == 27) SetMenuEdit(-1, -1, th);
	else if (ch == '\r') ApplyMenuEdit(th);
	else if (ch == 8) {num /= 10; Paint(PaintWMenu | PaintWFast, th);}
	else if (ch >= '0' && ch <= '9')
	{
		num = 10 * num + (ch - '0');
		if (num >= menu.edit_num_max) num = menu.edit_num_max;
		Paint(PaintWMenu | PaintWFast, th);
	}
	else if (menu.edit == MenuISize)
	{
		if (ch == '\t') SetMenuEdit(MenuISize, !menu.edit_index, th);
		else if (ch == 'x' || ch == 'X') SetMenuEdit(MenuISize, 0, th);
		else if (ch == 'y' || ch == 'Y') SetMenuEdit(MenuISize, 1, th);
	}
}

void CalculateSpeed()
{
	double t = 0, g = 0, dn, n = 0, st = 0, sg = 0, ss = 0, sp = 0;
	int i = timegenpos;
	do
	{
		if (t >= 500) break;
		g += timegen[i].g;
		dn = timegen[i].g;
		n += dn * (500 - t);
		st += dn * t;
		sg += dn * g;
		ss += dn * t * t;
		sp += dn * g * t;
		if (--i < 0) i = TimeGenLength;
		t += timegen[i].t;
	} while(i != timegenpos);
	ss = n * ss - st * st;
	sp = n * sp - sg * st;
	if (st < 1e-4 || ss < 1e-4 * st * st) g = 0;
	else g = sp / ss;
	generate.speed = 100 * g;
}

void Paint(int what, TThreadData th)
{
	what |= generate.paint;
	if (!(what & PaintWAll) || !life_data || xpar.win <= 0 || ypar.win <= 0) return;

	const unsigned int FAST_PAINT_TIME = 2, WAIT_PAINT_TIME = 8;
	unsigned int t = Clock() - generate.paint_time;
	unsigned int wt = (what & PaintWFast) ? FAST_PAINT_TIME : WAIT_PAINT_TIME;
	if (!(what & PaintWNow) && t >= (unsigned int)(-WAIT_PAINT_TIME))
	{
		if ((unsigned int)(-t) > wt) generate.paint_time += t + wt;
		generate.paint = what;
		return;
	}
	generate.paint_time += t + wt;
	generate.paint = PaintWNull;

	if (!picture)
	{
		SetPictureSize();
		if (!picture) return;
	}
	if (what & PaintWPole)
	{
		const unsigned char bgcolor[3] = {128, 128, 0};
		int w, h, x, y, xx, yy, i, j;
		int menu0 = GetMenuYPos(), menu1 = menu0 + GetMenuHeight();
		unsigned char *p = picture;
		unsigned char *data0 = (unsigned char*)GetDataAddress(life_data);
		int size = GetDataSize(xpar.p, ypar.p);
		if (xpar.win <= 0 || ypar.win <= 0) return;
		if (mpar.hit_type > 0)
		{
			double shift_x = xpar.shift, shift_y = ypar.shift;
			ApplyHit(data0, data0 + size);
			data0 += size;
			GetPaintOrigin(w, h, x, y, xx, yy);
			xpar.shift = shift_x; ypar.shift = shift_y;
		}
		else GetPaintOrigin(w, h, x, y, xx, yy);
		APosPixel pixel(xpar.p, ypar.p, data0);
		j = menu0;
		if (j < 0) j = 0;
		else if (j > yy) j = yy;
		for (i = j * xpar.win; i > 0; i--)
		{
			*(p++) = bgcolor[0]; *(p++) = bgcolor[1]; *(p++) = bgcolor[2];
		}
		i = menu1;
		if (i < 0) i = 0;
		else if (i > yy) i = yy;
		p += 3 * xpar.win * (i - j);
		for (i = (yy - i) * xpar.win; i > 0; i--)
		{
			*(p++) = bgcolor[0]; *(p++) = bgcolor[1]; *(p++) = bgcolor[2];
		}
		for (i = 0; i < h; i++)
		{
			for (j = xx; j > 0; j--)
			{
				*(p++) = bgcolor[0]; *(p++) = bgcolor[1]; *(p++) = bgcolor[2];
			}
			pixel.SetTo(x, (y + i) % ypar.p);
			j = xpar.p - x;
			if (j > w) j = w;
			for (;;)
			{
				if (pixel.GetPixel()) {*(p++) = 255; *(p++) = 255; *(p++) = 255;}
				else {*(p++) = 0; *(p++) = 0; *(p++) = 0;}
				if (--j <= 0) break;
				pixel.AddX1();
			}
			j = w - (xpar.p - x);
			if (j > 0)
			{
				pixel.SetTo(0, (y + i) % ypar.p);
				for (;;)
				{
					if (pixel.GetPixel()) {*(p++) = 255; *(p++) = 255; *(p++) = 255;}
					else {*(p++) = 0; *(p++) = 0; *(p++) = 0;}
					if (--j <= 0) break;
					pixel.AddX1();
				}
			}
			for (j = xpar.win - xx - w; j > 0; j--)
			{
				*(p++) = bgcolor[0]; *(p++) = bgcolor[1]; *(p++) = bgcolor[2];
			}
		}
		j = menu0;
		if (j < yy + h) j = yy + h;
		else if (j > ypar.win) j = ypar.win;
		for (i = (j - yy - h) * xpar.win; i > 0; i--)
		{
			*(p++) = bgcolor[0]; *(p++) = bgcolor[1]; *(p++) = bgcolor[2];
		}
		i = menu1;
		if (i < yy + h) i = yy + h;
		else if (i > ypar.win) i = ypar.win;
		p += 3 * xpar.win * (i - j);
		for (i = (ypar.win - i) * xpar.win; i > 0; i--)
		{
			*(p++) = bgcolor[0]; *(p++) = bgcolor[1]; *(p++) = bgcolor[2];
		}
	}
	if (what & PaintWSpeed) CalculateSpeed();
	MenuPaint(what);
	SetPicture(picture, (unsigned short)xpar.win, (unsigned short)ypar.win, th);
}

bool KolibriOnStart(TStartData &me_start, TThreadData th)
{
	randomize();
	me_start.WinData.Title = "Black and white Life";
	me_start.Width = 500; me_start.Height = 400;
	InitGenerate();
	InitMenuButton();
	if(LoadLibrary("proc_lib.obj", library_path, "/sys/lib/proc_lib.obj", &import_proc_lib))
	{
		ofd.procinfo = procinfo;
		ofd.com_area_name = "FFFFFFFF_open_dialog";
		ofd.com_area = 0;
		ofd.opendir_path = plugin_path;
		ofd.dir_default_path = "/sys";
		ofd.start_path = "/sys/File managers/opendial";
		ofd.draw_window = DrawWindow;
		ofd.status = 0;
		ofd.openfile_path = CommandLine;
		ofd.filename_area = filename_area;
		ofd.filter_area = &filter1;
		ofd.x_size = 420;
		ofd.x_start = 10;
		ofd.y_size = 320;
		ofd.y_start = 10;
		OpenDialog_Init(&ofd);
	} else return false;
	if (CommandLine[0]) open_file_str = true;
	return true;
}

bool KolibriOnClose(TThreadData)
{
	SetPictureSize(0, 0);
	SetPoleSize(0, 0);
	return true;
}

int KolibriOnIdle(TThreadData th)
{
	static const unsigned int WAIT_TIME = 2, GEN_TIME = 1;
	int res = -1;
	if (open_file_str)
	{
		MenuOpenDialogEnd(th);
		res = 0;
	}
	else
	{
		if (life_data && menu.button[MenuIGenerate].check)
		{
			unsigned int t = Clock() - generate.time;
			if (t >= (unsigned int)(-WAIT_TIME)) res = -t;
			else
			{
				MoveGenerateTime(t);
				if (generate.count > 0)
				{
					unsigned char *data0 = (unsigned char*)GetDataAddress(life_data);
					int size = GetDataSize(xpar.p, ypar.p);
					int flag = (menu.button[MenuIHCircle].check ? 4 : 1) +
								(menu.button[MenuIVCircle].check ? 8 : 2);
					if (++timegenpos >= TimeGenLength) timegenpos = 0;
					timegen[timegenpos].t = 0;
					timegen[timegenpos].g = 0;
					for (;;)
					{
						OneGeneration(xpar.p, ypar.p, data0 + size, data0, flag);
						if (menu.button[MenuIRandom].check) RandomDraw(data0 + size);
						timegen[timegenpos].g++;
						if (--generate.count == 0 || (unsigned int)(Clock() - generate.time) >= GEN_TIME)
						{
							MemCopy(data0, data0 + size, size);
							break;
						}
						OneGeneration(xpar.p, ypar.p, data0, data0 + size, flag);
						if (menu.button[MenuIRandom].check) RandomDraw(data0);
						timegen[timegenpos].g++;
						if (--generate.count == 0 || (unsigned int)(Clock() - generate.time) >= GEN_TIME) break;
					}
					generate.paint |= PaintWPole | PaintWSpeed;
				}
				if (generate.stop && generate.count == 0)
				{
					ResetGenerate();
					menu.button[MenuIGenerate].check = false;
					generate.paint |= PaintWMenuBorder | PaintWSpeed;
					res = -1;
				}
				else
				{
					MoveGenerateTime(Clock() - generate.time);
					res = (generate.count <= generate.gps / 100) ? WAIT_TIME : 0;
					MoveGenerateTime(res);
				}
			}
		}
	}
	if (generate.paint)
	{
		Paint((res < 0 || res > WAIT_TIME) ? (PaintWNull | PaintWNow) : PaintWNull, th);
	}
	return res;
}

void KolibriOnSize(int window_rect[], Kolibri::TThreadData th)
{
	unsigned short w, h;
	GetClientSize(w, h, window_rect[2], window_rect[3], th);
	SetPictureSize(w, h);
	generate.paint |= PaintWAll | PaintWFast;
	if (!life_data) MenuWinSizeClick(th);
	Paint(PaintWNull | PaintWNow, th);
}

void KolibriOnKeyPress(TThreadData th)
{
	int ch;
	while ((ch = GetKey()) >= 0)
	{
		if (mpar.hit_type > 0)
		{
			mpar.hit_type = 0;
			generate.paint |= PaintWPole | PaintWFast;
			SetMenuPressed(-1, th);
			if (generate.paint) Paint(PaintWNull, th);
		}
		else if (menu.pressed >= 0) SetMenuPressed(-1, th);
		else if (menu.edit >= 0) MenuEditPressKey(ch, th);
		else
		{
			switch (ch)
			{
			case 'm':
			case 'M':
				SetMenuDraw(!menu.draw, th);
				break;
			case 'c':
			case 'C':
				MenuClearClick(th);
				break;
			case 'o':
			case 'O':
				ofd.type = 0; // 0 - open
				OpenDialog_Start(&ofd);
				if(ofd.status==1) open_file_str=true;
				break;
			case 'a':
			case 'A':
				MenuAboutClick(th);
				break;
			case 'q':
			case 'Q':
				CloseWindow(th);
				break;
			case 'w':
			case 'W':
				MenuWinSizeClick(th);
				break;
			case 'g':
			case 'G':
				MenuGenerateClick(th);
				break;
			case 'r':
			case 'R':
				menu.button[MenuIRandom].check = !menu.button[MenuIRandom].check;
				Paint(PaintWMenuBorder | PaintWFast, th);
				break;
			case 'v':
			case 'V':
				menu.button[MenuIVCircle].check = !menu.button[MenuIVCircle].check;
				Paint(PaintWMenuBorder | PaintWFast, th);
				break;
			case 'h':
			case 'H':
				menu.button[MenuIHCircle].check = !menu.button[MenuIHCircle].check;
				Paint(PaintWMenuBorder | PaintWFast, th);
				break;
			case 'l':
			case 'L':
				SetMenuCurrent(MenuILine, th);
				break;
			case 's':
			case 'S':
				SetMenuCurrent(MenuIScroll, th);
				break;
			case 'x':
			case 'X':
				if (!menu.draw) SetMenuDraw(true, th);
				SetMenuEdit(MenuISize, 0, th);
				break;
			case 'y':
			case 'Y':
				if (!menu.draw) SetMenuDraw(true, th);
				SetMenuEdit(MenuISize, 1, th);
				break;
			case 'f':
			case 'F':
				if (!menu.draw) SetMenuDraw(true, th);
				SetMenuEdit(MenuISpeed, 0, th);
				break;
			case ' ':
			case '\\':
			case '|':
				if (menu.button[MenuIGenerate].check && !generate.stop)
				{
					menu.button[MenuIGenerate].check = false;
					Paint(PaintWMenuBorder | PaintWSpeed | PaintWFast, th);
				}
				else AddGenerateCount((ch == ' ') ? 1 : 15);
				break;
			}
		}
	}
}

void KolibriOnMouse(TThreadData th)
{
	short xp = 0, yp = 0;
	int w, h, x, y, xx, yy, m;
	GetMousePosPicture(xp, yp);
	m = GetMouseButton();
	GetPaintOrigin(w, h, x, y, xx, yy);
	x += xp - xx;
	y += yp - yy;
	if (mpar.hit_type > 0)
	{
		if (mpar.last_x != x || mpar.last_y != y)
		{
			mpar.last_x = x; mpar.last_y = y;
			generate.paint |= PaintWPole | PaintWFast;
		}
		if (m != mpar.button)
		{
			if ((m & ~mpar.button) == 0) ApplyHit();
			mpar.hit_type = 0;
			generate.paint |= PaintWPole | PaintWFast;
			SetMenuPressed(-1, th);
			if (generate.paint) Paint(PaintWNull, th);
		}
	}
	else if (menu.pressed >= 0)
	{
		if (mpar.last_x != x || mpar.last_y != y)
		{
			MenuMouseClick(xp, yp, -1, th);
		}
		if (m != mpar.button)
		{
			if ((m & ~mpar.button) == 0) MenuMouseClick(xp, yp, 0, th);
			SetMenuPressed(-1, th);
		}
	}
	else if (mpar.button == 0 && (m == 1 || m == 2))
	{
		if (xp >= 0 && xp < xpar.win && yp >= 0 && yp < ypar.win)
		{
			mpar.hit_x = x; mpar.hit_y = y; mpar.last_x = x; mpar.last_y = y;
			yy = GetMenuYPos();
			if (menu.edit >= 0 || (yp >= yy && yp < yy + GetMenuHeight()))
			{
				MenuMouseClick(xp, yp, m, th);
			}
			else PoleMouseClick(m, th);
		}
	}
	mpar.button = m;
}

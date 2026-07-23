#include "func.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>

int convert_error = 0;

double convert(char *s, int *len)
{
	int i = 0;
	double sign, res = 0.0, tail, div;

	convert_error = 0;

	while (s[i] && isspace((unsigned char)s[i]))
		i++;
	if (len)
		*len = i;
	if (s[i] == '\0') {
		convert_error = ERROR_END;
		return 0.0;
	}

	sign = 1.0;
	if (s[i] == '-') {
		sign = -1.0;
		i++;
	}

	while (s[i] >= '0' && s[i] <= '9') {
		res = res * 10.0 + (double)(s[i] - '0');
		i++;
	}
	if (len)
		*len = i;
	if (!s[i] || isspace((unsigned char)s[i]))
		return sign * res;
	if (s[i] != '.' && s[i] != ',') {
		convert_error = ERROR;
		return 0;
	}
	i++;
	if (len)
		*len = i;
	if (!s[i])
		return sign * res;

	div = 1.0;
	tail = 0.0;
	while (s[i] >= '0' && s[i] <= '9') {
		tail = tail * 10.0 + (double)(s[i] - '0');
		div *= 10.0;
		i++;
	}
	res += tail / div;
	if (len)
		*len = i;
	return sign * res;
}

char *ftoa(double d)
{
	char buffer[256];
	char *p;
	sprintf(buffer, "%f", d);
	p = malloc(strlen(buffer) + 1);
	if (p)
		strcpy(p, buffer);
	return p;
}

int strnicmp(const char *a, const char *b, unsigned n)
{
	while (n--) {
		int ca = tolower((unsigned char)*a++);
		int cb = tolower((unsigned char)*b++);
		if (ca != cb)
			return ca - cb;
		if (!ca)
			break;
	}
	return 0;
}

void draw_text(int x, int y, unsigned font, uint32_t color, const char *s, int len)
{
	__asm__ __volatile__(
		"int $0x40"
		:
		: "a"(4), "b"((x << 16) | (y & 0xFFFF)),
		  "c"((font << 24) | (color & 0xFFFFFF)), "d"(s), "S"(len)
		: "memory");
}

void draw_line(int x1, int y1, int x2, int y2, uint32_t color, int invert)
{
	uint32_t edx = invert ? 0x01000000 : color;
	__asm__ __volatile__(
		"int $0x40"
		:
		: "a"(38), "b"((x1 << 16) | (x2 & 0xFFFF)),
		  "c"((y1 << 16) | (y2 & 0xFFFF)), "d"(edx)
		: "memory");
}

void draw_region(int x, int y, int w, int h, uint32_t color)
{
	_ksys_draw_line(x, y, x + w - 2, y, color);
	_ksys_draw_line(x, y + 1, x, y + h - 1, color);
	_ksys_draw_line(x + w - 1, y, x + w - 1, y + h - 2, color);
	_ksys_draw_line(x + 1, y + h - 1, x + w - 1, y + h - 1, color);
}

void draw_cut_text(int x, int y, int area_w, uint32_t color, const char *s)
{
	int len;
	if (!s)
		return;
	len = strlen(s);
	if (len * 8 > area_w)
		len = area_w / 8;
	draw_text(x, y, 0x10, color, s, len);
}

#include "encoding.h"

// the letters of cp866 0xF0..0xF7: Ё ё Є є Ї ї Ў ў
static const unsigned short cp866_f0[8] = { 0x401, 0x451, 0x404, 0x454, 0x407, 0x457, 0x40E, 0x45E };

// the character of a cp866 byte, '?' if it has none
static unsigned cp866_char(unsigned char c)
{
	if (c < 0x80)
		return c;
	if (c < 0xB0)                   // А..п
		return c - 0x80 + 0x410;
	if (c >= 0xE0 && c < 0xF0)      // р..я
		return c - 0xE0 + 0x440;
	if (c >= 0xF0 && c < 0xF8)
		return cp866_f0[c - 0xF0];
	return '?';
}

// a character in cp866: the byte cp866_char() gives it for, '?' if none
static unsigned char unicode_to_cp866(unsigned c)
{
	int b;

	if (c < 0x80)
		return c;
	for (b = 0x80; b <= 0xFF; b++)
		if (cp866_char(b) == c)
			return b;
	return '?';
}

// the bytes after a lead byte of UTF-8; -1 if it is none
static int utf8_more(unsigned char c)
{
	if (c < 0x80)
		return 0;
	if (c < 0xC0) // 10xxxxxx: a byte after one
		return -1;
	if (c < 0xE0)
		return 1;
	if (c < 0xF0)
		return 2;
	if (c < 0xF8)
		return 3;
	return -1;
}

#define NOT_UTF8 0xFFFFFFFF // what utf8_next() gives for bytes that are not UTF-8

// the character at *p, *p moved past it; NOT_UTF8 for a byte that starts
// none, a character cut short, or one in more bytes than it needs ("C0 80"
// for a zero would end the text)
static unsigned utf8_next(const unsigned char **p)
{
	static const unsigned least[4] = { 0, 0x80, 0x800, 0x10000 }; // in 1, 2, 3, 4 bytes
	const unsigned char *s = *p;
	unsigned c = *s++;
	int more = utf8_more(c), left;

	if (more < 0) {
		*p = s;
		return NOT_UTF8;
	}
	if (more)
		c &= 0x3F >> more; // the bits of the character in the lead byte
	for (left = more; left && (*s & 0xC0) == 0x80; left--)
		c = c << 6 | (*s++ & 0x3F);
	*p = s;
	return left || c < least[more] ? NOT_UTF8 : c;
}

void utf8_to_cp866(const char *s, char *out, int size)
{
	const unsigned char *p = (const unsigned char *)s;

	while (*p && size > 1) {
		unsigned c = utf8_next(&p);
		*out++ = c == NOT_UTF8 ? '?' : unicode_to_cp866(c);
		size--;
	}
	*out = '\0';
}

int cp866_to_utf8(unsigned char c, char *out)
{
	unsigned u = cp866_char(c);

	if (u < 0x80) {
		*out = u;
		return 1;
	}
	out[0] = 0xC0 | u >> 6;
	out[1] = 0x80 | (u & 0x3F);
	return 2;
}

int is_utf8(const char *s)
{
	const unsigned char *p = (const unsigned char *)s;

	while (*p) {
		unsigned c = utf8_next(&p);
		if (c == NOT_UTF8 || (c >= 0x80 && unicode_to_cp866(c) == '?'))
			return 0;
	}
	return 1;
}

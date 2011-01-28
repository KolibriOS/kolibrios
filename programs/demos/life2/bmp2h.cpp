#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "picture.h"

char str[1000], name[1000], *s;
int type;

int mx = 0, my = 0, slen;
BYTE *tex = 0;

inline void out_uc(unsigned char c, int &k)
{
	if (k < 0) {printf("\n\""); k = 0;}
	printf("\\x%02X", c);
	k++;
	if (k >= 20) {printf("\""); k = -1;}
}

inline void out_cl(int &k)
{
	if (k >= 0) {printf("\""); k = -1;}
}

inline void out_end(int &k)
{
	if (k == -10) return;
	if (type == 0) {out_uc(0, k); out_uc(255, k);}
	out_cl(k);
	printf(";\n\n");
	k = -10;
}

void del_last_space(char *s0)
{
	char *s1;
	for (s1 = s0; *s1; s1++)
	{
		if (!isspace(*s1)) s0 = s1 + 1;
	}
	*s0 = 0;
}

int main()
{
	BYTE *t0;
	int i, j, k, m, t, v;
//	randomize();
	srand(time(NULL));
	i = rand() + (rand() << 16);
	j = rand() + (rand() << 16);
	k = rand() + (rand() << 16);
	printf("#ifndef _INCLUDE_BITMAP_%08X_%08X_%08X__PICTURE_\n", i, j, k);
	printf("#define _INCLUDE_BITMAP_%08X_%08X_%08X__PICTURE_\n", i, j, k);
	strcpy(name, "bitmap_pictures");
	type = 0;
	k = -10;
	while (!feof(stdin))
	{
		memset(str, 0, sizeof(str));
		fgets(str, sizeof(str)-1, stdin);
		s = str;
		while (*s && isspace(*s)) s++;
		if (strncmp(s, "rem", 3) != 0) continue;
		s += 3;
		if (!isspace(*s)) continue;
		while (*s && isspace(*s)) s++;
		if (strncmp(s, "<array", 6) == 0)
		{
			s += 6;
			if (strncmp(s, ":bmp>", 5) == 0) {s += 5; v = 0;}
			else if (strncmp(s, ":set>", 5) == 0) {s += 5; v = 1;}
			else if (strncmp(s, ":life>", 6) == 0) {s += 6; v = 2;}
			else continue;
			if (!isspace(*s)) continue;
			while (*s && isspace(*s)) s++;
			if (!*s) continue;
			del_last_space(s);
			strcpy(name, s);
			out_end(k);
			type = v;
			continue;
		}
		else if (strncmp(s, "<skip>", 6) == 0)
		{
			s += 6;
			if (!isspace(*s)) continue;
			while (*s && isspace(*s)) s++;
			if (!*s) continue;
			i = 2*atoi(s);
			while (--i >= 0) out_uc(0, k);
			continue;
		}
		else if (strncmp(s, "<bitmap", 7) == 0)
		{
			s += 7; v = 0;
			for (;;)
			{
				if (strncmp(s, ":inv", 4) == 0) {s += 4; v ^= 1;}
				else break;
			}
			if (s[0] == '>') s++;
			else continue;
		}
		else continue;
		if (!isspace(*s)) continue;
		while (*s && isspace(*s)) s++;
		if (!*s) continue;
		del_last_space(s);
		if ((PictureFileOpen(mx, my, tex, s) & PFO_MASK_ERROR) != 0) continue;
		if (!tex || mx <= 0 || my <= 0) continue;
		if (type == 0 || type == 1)
		{
			if (mx >= 256 || my >= 256) continue;
		}
		else if (type == 2)
		{
			if (mx >= 65536 || my >= 65536) continue;
		}
		slen = GetStringPictLenght(mx);
		if (k == -10)
		{
			printf("\nconst unsigned char %s[] = \"", name);
			if (type == 2) printf("#LifeBin 2.0\\n");
			printf("\"");
			k = -1;
		}
		if (v & 1)
		{
			for (i = 0; i < my; i++)
			{
				t0 = tex + slen * i;
				for (j = 3*mx; j > 0; j--)
				{
					t0[0] = (unsigned char)~t0[0];
					t0++;
				}
			}
		}
		m = 0; t = 0;
		if (type == 0 || type == 1)
		{
			out_uc((unsigned char)mx, k); out_uc((unsigned char)my, k);
		}
		else if (type == 2)
		{
			out_uc((unsigned char)mx, k); out_uc((unsigned char)(mx >> 8), k);
			out_uc((unsigned char)my, k); out_uc((unsigned char)(my >> 8), k);
		}
		for (i = 0; i < my; i++)
		{
			t0 = tex + slen * i;
			for (j = mx; j > 0; j--)
			{
				if (type == 0)
				{
					out_uc(t0[2], k); out_uc(t0[1], k); out_uc(t0[0], k);
				}
				else if (type == 1)
				{
					m |= ((int)t0[0] + (int)t0[1] + (int)t0[2] >= 384) << t;
					if (++t >= 8) {out_uc((unsigned char)m, k); m = 0; t = 0;}
				}
				else if (type == 2)
				{
					if ((int)t0[0] + (int)t0[1] + (int)t0[2] >= 384)
					{
						if (m) {out_uc((unsigned char)m, k); m = 0;}
						out_uc(0, k);
					}
					else
					{
						if (m == 255) {out_uc((unsigned char)m, k); m = 0;}
						m++;
					}
				}
				t0 += 3;
			}
		}
		if (type == 1)
		{
			if (t) out_uc((unsigned char)m, k);
		}
		else if (type == 2)
		{
			if (m) out_uc((unsigned char)m, k);
		}
	}
	if (tex) delete[] tex;
	out_end(k);
	printf("#endif\n");
	return 0;
}
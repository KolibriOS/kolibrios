#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <cstring>
#include <string>
using namespace std;

char str[20000], *word[100];
int wordlen[sizeof(word) / sizeof(word[0])];

char words_ignore[100][100] = { "?debug",
								"if",
								"ifdef",
								"ifndef",
								"endif",
								"macro",
								"endm",
								"dgroup",
								"assume",
								"segment",
								"ends",
								"public",
								"proc",
								"endp",
								"extrn",
								"model",
								"label",
								"end",
								""};

#define WI_macro    (words_ignore[5])
#define WI_endm     (words_ignore[6])
#define WI_segment  (words_ignore[9])
#define WI_ends     (words_ignore[10])
#define WI_proc     (words_ignore[12])
#define WI_endp     (words_ignore[13])
#define WI_label    (words_ignore[16])
const char *WI_END = "END";

int nwords_ignore = -1;
char *words_ign_sort[sizeof(words_ignore) / sizeof(words_ignore[0])];

int strcmp1(const char *s0, int n0, const char *s1, int n1)
{
	while (n0 && n1)
	{
		n0--; n1--;
		int c = (int)tolower(*s0) - (int)tolower(*s1);
		if (c < 0) return -1;
		if (c > 0) return 1;
		if (!*(s0++) || !*(s1++)) return 0;
	}
	if (n0 && *s0) return 1;
	if (n1 && *s1) return -1;
	return 0;
}

int strlen1(const char *s, int n)
{
	int i = 0;
	while (i < n && s[i]) i++;
	return i;
}

char *strcpy1(char *r, const char *s, int n)
{
	char *r0 = r;
	while (n-- && *s) *(r++) = *(s++);
	*r = 0;
	return r0;
}

char *strchr1(char *s, int n, char c)
{
	while (n-- && *s)
	{
		if (*s == c) return s;
		s++;
	}
	return 0;
}

char *first_not_space(char *s)
{
	while (*s && isspace(*s)) s++;
	return s;
}

char get_word(char *s)
{
	int i = 0, k;
	for (k = 0; k < sizeof(word) / sizeof(word[0]); k++)
	{
		s = first_not_space(s + i);
		word[k] = s;
		i = 0;
		while (s[i] && !isspace(s[i]) && s[i] != ';' && s[i] != ',' && s[i] != ':')
		{
			i++;
		}
		if (i == 0)
		{
			if (s[i] != ',' && s[i] != ':') break;
			i = 1;
		}
		wordlen[k] = i;
	}
	for (; k < sizeof(word) / sizeof(word[0]); k++)
	{
		word[k] = s; wordlen[k] = 0;
	}
	return s[i];
}

void build_words_ignore()
{
	int i, j;
	nwords_ignore = 0;
	while (words_ignore[nwords_ignore][0])
	{
		words_ign_sort[nwords_ignore] = words_ignore[nwords_ignore];
		nwords_ignore++;
	}
	for (i = 0; i < nwords_ignore; i++) for (j = 0; j < i; j++)
	{
		if (strcmp1(words_ign_sort[j], -1, words_ign_sort[i], -1) > 0)
		{
			char *s = words_ign_sort[j];
			words_ign_sort[j] = words_ign_sort[i];
			words_ign_sort[i] = s;
		}
	}
	words_ign_sort[nwords_ignore] = words_ignore[nwords_ignore];
}

int find_word_ign_sort(const char *s, int n)
{
	int i0, i1, i;
	if (nwords_ignore < 0) build_words_ignore();
	i0 = -1; i1 = nwords_ignore;
	while (i1 > i0 + 1)
	{
		i = (i0 + i1) / 2;
		if (strcmp1(s, n, words_ign_sort[i], -1) > 0) i0 = i;
		else i1 = i;
	}
	return i1;
}

char *find_word_ignore(const char *s, int n)
{
	int i = find_word_ign_sort(s, n);
	return (strcmp1(s, n, words_ign_sort[i], -1) == 0) ? words_ign_sort[i] : 0;
}

char *add_word_ignore(const char *s, int n)
{
	int i = find_word_ign_sort(s, n), j;
	if (strcmp1(s, n, words_ign_sort[i], -1) != 0)
	{
		if (s[0] && strlen1(s, n) < sizeof(words_ignore[0]) &&
			words_ignore[nwords_ignore+1] < words_ignore[0] + sizeof(words_ignore))
		{
			strcpy1(words_ignore[nwords_ignore], s, n);
			words_ignore[nwords_ignore+1][0] = 0;
			for (j = nwords_ignore; j >= i; j--) words_ign_sort[j+1] = words_ign_sort[j];
			words_ign_sort[i] = words_ignore[nwords_ignore];
			nwords_ignore++;
			words_ign_sort[nwords_ignore] = words_ignore[nwords_ignore];
		}
		else return 0;
	}
	return words_ign_sort[i];
}

struct CSegment
{
	char name[100];
	int state;
	string text;
};

int nsegment = 0, cursegment = 0, stack_segment[50], nstack_segment = 0;
CSegment segment[100];

void push_stack_segment()
{
	if (nstack_segment < sizeof(stack_segment) / sizeof(stack_segment[0]))
	{
		stack_segment[nstack_segment] = cursegment;
	}
	nstack_segment++;
}

void pop_stack_segment()
{
	if (nstack_segment > 0) nstack_segment--;
	if (nstack_segment < sizeof(stack_segment) / sizeof(stack_segment[0]))
	{
		cursegment = stack_segment[nstack_segment];
	}
}

void newsegment()
{
	segment[nsegment].name[0] = 0;
	segment[nsegment].state = 0;
	segment[nsegment].text = "";
	cursegment = nsegment++;
}

void tosegment(char *name, int nameL)
{
	int i;
	cursegment = 0;
	if (strlen1(name, nameL) >= sizeof(segment[0].name)) return;
	for (i = 0; i < nsegment; i++)
	{
		if (strcmp1(name, nameL, segment[i].name, -1) == 0)
		{
			cursegment = i;
			return;
		}
	}
	if (nsegment >= sizeof(segment) / sizeof(segment[0])) return;
	newsegment();
	strcpy1(segment[cursegment].name, name, nameL);
}

void outsegment()
{
	if (segment[cursegment].state == 0) return; 
	fputs("\nsegment", stdout);
	if (segment[cursegment].name[0])
	{
		fputc(' ', stdout);
		fputs(segment[cursegment].name, stdout);
	}
	fputc('\n', stdout);
	fputs(segment[cursegment].text.c_str(), stdout);
	fputs("endseg", stdout);
	if (segment[cursegment].name[0])
	{
		fputs("  ", stdout);
		fputs(segment[cursegment].name, stdout);
	}
	fputc('\n', stdout);
}

void transform_label(int in_define, bool label = false)
{
	if (in_define)
	{
		memmove(str + 8, word[0], wordlen[0]);
		memcpy(str, "nextdef ", 8);
		str[wordlen[0]+8] = '\n';
		str[wordlen[0]+9] = 0;
	}
	else if (label)
	{
		memmove(str, word[0], wordlen[0]);
		str[wordlen[0]] = ':';
		str[wordlen[0]+1] = '\n';
		str[wordlen[0]+2] = 0;
	}
	segment[cursegment].state |= 4;
}

int main()
{
	char *s;
	int in_macro = 0, in_define = 0, i;
	newsegment();
	for (;;)
	{
		i = sizeof(str) - 200;
		if (!fgets(str, i, stdin)) break;
		if ((s = strchr(str, '\n')) == NULL)
		{
			if (strlen(str) < i-1) strcat(str, "\n");
			else
			{
				while (fgets(str, sizeof(str), stdin))
				{
					if (strchr(str, '\n')) break;
				}
				continue;
			}
		}
		char* p;
		while (p=strstr(str,"st("))
		{
			p[2]=p[3];
			memmove(p+3,p+5,strlen(p+5)+1);
		}
		get_word(str);
		if (word[1][0] == ':') transform_label(in_define, false);
		else
		{
			if (word[0][0] == '.') continue;
			if (strcmp1(word[0], wordlen[0], WI_macro, -1) == 0 ||
				strcmp1(word[1], wordlen[1], WI_macro, -1) == 0)
			{
				add_word_ignore(word[0], wordlen[0]);
				in_macro++;
				continue;
			}
			if (strcmp1(word[0], wordlen[0], WI_endm, -1) == 0)
			{
				if (in_macro > 0) in_macro--;
				continue;
			}
			if (strcmp1(word[1], wordlen[1], WI_segment, -1) == 0)
			{
				push_stack_segment();
				add_word_ignore(word[0], wordlen[0]);
				tosegment(word[0], wordlen[0]);
				continue;
			}
			if (strcmp1(word[1], wordlen[1], WI_ends, -1) == 0)
			{
				pop_stack_segment();
				continue;
			}
			if (find_word_ignore(word[0], wordlen[0])) continue;
			if (wordlen[0] == 0 || strcmp1(word[0], wordlen[0], "align", -1) == 0)
			{
				if (word[0][0] == 0) {str[0] = '\n'; str[1] = 0;}
			}
			else if (strcmp1(word[1], wordlen[1], WI_endp, -1) == 0)
			{
				if (in_define > 0) strcpy(str, (--in_define) ? "newdef\n" : "enddef\n");
			}
			else if (strcmp1(word[1], wordlen[1], WI_proc, -1) == 0)
			{
				memmove(str + 8, word[0], wordlen[0]);
				memcpy(str, (in_define++) ? "newdef  " : "define  ", 8);
				str[wordlen[0]+8] = '\n';
				str[wordlen[0]+9] = 0;
				segment[cursegment].state |= 4;
			}
			else if (strcmp1(word[1], wordlen[1], WI_label, -1) == 0)
			{
				transform_label(in_define, true);
			}
			else
			{
				for (i = 0; i <= 1; i++)
				{
					if (strcmp1(word[i], wordlen[i], "db", -1) == 0 &&
						wordlen[i+2] >= 3 && strcmp1(word[i+2], 3, "dup", 3) == 0)
					{
						if (word[i][0] == 'd') word[i][0] = 'r';
						else if (word[i][0] == 'D') word[i][0] = 'R';
						word[i+1][wordlen[i+1]] = '\n';
						word[i+1][wordlen[i+1]+1] = 0;
						segment[cursegment].state |= 2;
						break;
					}
				}
				if (i > 1)
				{
					int word_shift = 0;
					for (i = 1; i < sizeof(word) / sizeof(word[0]); i++)
					{
						word[i] += word_shift;
						if (wordlen[i] >= 4 && word[i][0] != '[' &&
							word[i][wordlen[i]-1] == ']' &&
							(s = strchr1(word[i], wordlen[i], '[')) != 0)
						{
							*s = '+';
							memmove(word[i] + 1, word[i], strlen(word[i]) + 1);
							word[i][0] = '[';
							wordlen[i]++;
							word_shift++;
						}
						else if (wordlen[i] >= 1 && !strchr1(word[i], wordlen[i], '[') &&
								strcmp1(word[i-1], wordlen[i-1], "ptr", -1) == 0)
						{
							memmove(word[i] + wordlen[i] + 2, word[i] + wordlen[i],
									strlen(word[i] + wordlen[i]) + 1);
							memmove(word[i] + 1, word[i], wordlen[i]);
							word[i][0] = '['; word[i][wordlen[i] + 1] = ']';
							wordlen[i] += 2;
							word_shift += 2;
						}
						else if (wordlen[i] >= 5 && word[i][wordlen[i]-1] == ')' &&
									 strcmp1(word[i], wordlen[i], "st(", -1))
						{
							memmove(word[i] + 2, word[i] + 3, wordlen[i] - 4);
							memmove(word[i] + wordlen[i] - 2, word[i] + wordlen[i],
									strlen(word[i] + wordlen[i]) + 1);
							word_shift -= 2;
						}
					}
					segment[cursegment].state |= 1;
				}
			}
		}
		if (in_macro) continue;
		if ((s = strchr(str, ';')) != NULL)
		{
			s[0] = '\n'; s[1] = 0;
			if (!first_not_space(str)[0]) continue;
		}
		segment[cursegment].text += str;
	}
	for (cursegment = 0; cursegment < nsegment; cursegment++)
	{
		if ((segment[cursegment].state & 3) != 2) outsegment();
	}
	fputs("\nI_END:\n", stdout);
	for (cursegment = 0; cursegment < nsegment; cursegment++)
	{
		if ((segment[cursegment].state & 3) == 2) outsegment();
	}
	fputs("\nalign 0x10\nU_END:\n", stdout);
	return 0;
}


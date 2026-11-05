#include "texts.h"

#include <stdlib.h>
#include <string.h>

#define SMALLEST 16    // bytes of the smallest text; SMALLEST << i the others
#define SIZES    5     // 16 .. 256: CELL_MAX and its zero
#define CHUNK    16384 // bytes the kernel gives at a time, cut into texts of one size

// a free text: the next free one of its size in its first bytes
typedef struct Free {
	struct Free *next;
} Free;

static Free *free_texts[SIZES];

// i of the size SMALLEST << i a text of size bytes takes
static int size_index(int size)
{
	int i = 0;

	while ((SMALLEST << i) < size)
		i++;
	return i;
}

char *text_alloc(int size)
{
	int i = size_index(size), bytes = SMALLEST << i, k;
	Free *t;

	if (!free_texts[i]) { // a new chunk, all of it free texts of this size
		char *chunk = malloc(CHUNK);
		if (!chunk)
			return NULL;
		for (k = 0; k < CHUNK; k += bytes) {
			t = (Free *)(chunk + k);
			t->next = free_texts[i];
			free_texts[i] = t;
		}
	}
	t = free_texts[i];
	free_texts[i] = t->next;
	return (char *)t;
}

char *text_copy(const char *s)
{
	char *t = text_alloc(strlen(s) + 1);

	if (t)
		strcpy(t, s);
	return t;
}

void text_free(char *s)
{
	Free *t = (Free *)s;
	int i;

	if (!s)
		return;
	i = size_index(strlen(s) + 1); // before its first bytes are the link
	t->next = free_texts[i];
	free_texts[i] = t;
}

#ifndef SHEET_TEXTS_H
#define SHEET_TEXTS_H

// The memory of the texts of the cells. malloc() of libc.obj is a call of
// the kernel (function 68.12): two pages for each block at least, and a
// look through all the blocks given on each call; a sheet of short texts
// would take 8 KB a text, and be slow to fill and to free. Here a text
// takes 16, 32, 64, 128 or 256 bytes of a chunk, as its length needs;
// freed, it waits for the next text of its size. The chunks stay.

// size bytes, at most CELL_MAX + 1; NULL if no memory
char *text_alloc(int size);
// a copy of s; NULL if no memory
char *text_copy(const char *s);
// a text of text_alloc() or text_copy(), or NULL; its size is the one its
// length needs, so a text keeps the length it was made for
void text_free(char *s);

#endif

// The look of the windows: the skin, the font, the icons, the texts, the
// windows of the threads.

#include "look.h"

#include <string.h>

#define CHILD_STYLE 0x34 // skinned, fixed, caption, client coordinates
#define POPUP_STYLE 0x01 // the window only takes its place, the thread draws it all
#define ICON_WHITE 0xFFFFFFFF // in the colour under the icon
#define ICON_SHADE 0xFFCACBD6 // in work_dark, as @RESHARE makes ICONS18W

Skin sc;

void read_skin(void)
{
	_ksys_get_system_colors((ksys_colors_table_t *)&sc);
}

void draw_text(const char *s, int x, int y, uint32_t color)
{
	_ksys_draw_text(s, x, y, 0, FONT_UTF8_Z | color);
}

void draw_bevel(int x, int y, int w, int h, uint32_t top_left, uint32_t bottom_right)
{
	_ksys_draw_bar(x, y, w, 1, top_left);
	_ksys_draw_bar(x, y, 1, h, top_left);
	_ksys_draw_bar(x, y + h - 1, w, 1, bottom_right);
	_ksys_draw_bar(x + w - 1, y, 1, h, bottom_right);
}

int in_rect(int x, int y, int left, int top, int right, int bottom)
{
	return x >= left && x < right && y >= top && y < bottom;
}

#define TITLE_X   (GROUP_LINES + GAP - 1) // the letters of a group title, with their empty column
#define TITLE_PAD 7                       // on work at both sides of them

void draw_group(int x, int y, int w, int h, const char *s)
{
	int tx = x + TITLE_X, ty = y - GROUP_TITLE_DY;

	draw_bevel(x, y, w, h, sc.work_graph, sc.work_graph);
	draw_bevel(x + 1, y + 1, w - 2, h - 2, sc.work_light, sc.work_light);
	_ksys_draw_bar(tx - TITLE_PAD, ty, utf8_chars(s) * FONT_W + 2 * TITLE_PAD, FONT_H, sc.work);
	draw_text(s, tx, ty, sc.work_text);
}

/* ============================== icons ============================= */

// ICONS18, the memory @RESHARE shares: 18x18 ARGB each, white their
// background, a light grey their shadow
static uint32_t *icons;
static int icon_count;

// an icon at x, y on a background of colour back, as one image
// (function 7: 3 bytes a pixel, blue first); none without @RESHARE, which
// may start after the program: it is looked for again until it runs
void draw_icon(int icon, int x, int y, uint32_t back)
{
	uint8_t image[ICON_SIZE * ICON_SIZE * 3];
	const uint32_t *p;
	char *shm;
	int i, size;

	if (!icons) {
		size = _ksys_shm_open("ICONS18", KSYS_SHM_READ, 0, &shm);
		icons = (uint32_t *)shm;
		// function 68.22 gives the size, or a small error code: no icons
		icon_count = size / (ICON_SIZE * ICON_SIZE * 4);
	}
	if (icon >= icon_count)
		return;
	p = icons + icon * ICON_SIZE * ICON_SIZE;
	for (i = 0; i < ICON_SIZE * ICON_SIZE; i++) {
		uint32_t c = p[i];
		if (c == ICON_WHITE)
			c = back;
		else if (c == ICON_SHADE)
			c = sc.work_dark;
		image[i * 3] = c;
		image[i * 3 + 1] = c >> 8;
		image[i * 3 + 2] = c >> 16;
	}
	_ksys_draw_bitmap(image, x, y, ICON_SIZE, ICON_SIZE);
}

/* ============================== texts ============================= */

// the texts one after the other, each ended by a zero, in the order of T_*
#if defined LANG_RUS
static const char lang_texts[] =
	"Таблица " VERSION "\0" "Новая\0" "Открыть\0" "Сохранить\0"
	"Не удалось открыть файл\0" "Не удалось открыть весь файл\0" "Не удалось сохранить файл\0"
	"Не сохранять\0" "Отмена\0" "сохранён\0"
	"Меню\0" "Вырезать\0" "Копировать\0" "Вставить\0" "Очистить\0" "Отменить\0" "Повторить\0" "Справка\0"
	"Вставить слева\0" "Вставить справа\0" "Вставить выше\0" "Вставить ниже\0" "Удалить\0"
	"Арифметика\0" "Логика\0" "Ячейки\0" "Функции\0" "Математика\0" "Тригонометрия\0"
	"сдвигается при копировании\0" "не сдвигается\0" "не сдвигается столбец / строка\0" "ячейки с A1 по B3\0" "1, если число\0";
#elif defined LANG_SPA
static const char lang_texts[] =
	"Tabla " VERSION "\0" "Nueva\0" "Abrir\0" "Guardar\0"
	"No se puede abrir el archivo\0" "No se puede abrir todo el archivo\0" "No se puede guardar el archivo\0"
	"No guardar\0" "Cancelar\0" "guardado\0"
	"Menú\0" "Cortar\0" "Copiar\0" "Pegar\0" "Borrar\0" "Deshacer\0" "Rehacer\0" "Ayuda\0"
	"Insertar a la izquierda\0" "Insertar a la derecha\0" "Insertar arriba\0" "Insertar abajo\0" "Eliminar\0"
	"Aritmética\0" "Lógica\0" "Celdas\0" "Funciones\0" "Matemáticas\0" "Trigonometría\0"
	"cambia al copiar\0" "no cambia\0" "no cambia la columna / fila\0" "celdas de A1 a B3\0" "1 si es un número\0";
#else
static const char lang_texts[] =
	"Table " VERSION "\0" "New\0" "Open\0" "Save\0"
	"Cannot open the file\0" "Cannot open all of the file\0" "Cannot save the file\0"
	"Don't save\0" "Cancel\0" "is saved\0"
	"Menu\0" "Cut\0" "Copy\0" "Paste\0" "Clear\0" "Undo\0" "Redo\0" "Help\0"
	"Insert left\0" "Insert right\0" "Insert above\0" "Insert below\0" "Delete\0"
	"Arithmetic\0" "Logic\0" "Cells\0" "Functions\0" "Math\0" "Trigonometry\0"
	"shifts when copied\0" "does not shift\0" "column / row does not shift\0" "cells A1 to B3\0" "1 if a number\0";
#endif

const char *texts[T_COUNT];

void texts_init(void)
{
	const char *s = lang_texts;
	int i;

	for (i = 0; i < T_COUNT; i++) {
		texts[i] = s;
		s += strlen(s) + 1;
	}
}

int utf8_chars(const char *s)
{
	int n = 0;

	for (; *s; s++)
		n += (*s & 0xC0) != 0x80;
	return n;
}

/* ============================= windows ============================ */

void work_area(Area *a)
{
	uint32_t x_range, y_range; // function 48.5: left << 16 | right, top << 16 | bottom

	asm volatile("int $0x40" : "=a"(x_range), "=b"(y_range) : "a"(48), "b"(5));
	a->left = x_range >> 16;
	a->top = y_range >> 16;
	a->w = (x_range & 0xFFFF) - a->left + 1;
	a->h = (y_range & 0xFFFF) - a->top + 1;
}

void child_window(int w, int h, int title)
{
	Area a;

	work_area(&a);
	w += 2 * SKIN_BORDER;
	h += _ksys_get_skin_height() + SKIN_BORDER;
	_ksys_create_window(a.left + (a.w - w) / 2, a.top + (a.h - h) / 2, w, h, NULL, sc.work, CHILD_STYLE);
	child_title(title);
}

void popup_window(int x, int y, int w, int h, int title)
{
	_ksys_create_window(x, y, w, h, NULL, 0, POPUP_STYLE);
	child_title(title);
}

void window_title(const char *s, int encoding)
{
	asm volatile("int $0x40" : : "a"(71), "b"(2), "c"(s), "d"(encoding) : "memory");
}

void child_title(int text)
{
	window_title(texts[text], KSYS_TITLE_UTF8);
}


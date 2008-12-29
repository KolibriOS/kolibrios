
#include "func.h"
#include "parser.h"
#include "calc.h"
#include "use_library.h"
///#include "use_library.h"
//const char header[] = "Table";

#define TABLE_VERSION "0.94a"

// строки, которые выводит программа
const char *sFileSign = "KolibriTable File\n";
const char sFilename[] = "Filename: ";
const char sSave[] = "Save";
const char sLoad[] = "Load";
const char sNew[] = "New";

const char er_file_not_found[] = "Cannot open file. ";
const char er_format[] = "Error: bad format. ";
const char msg_save[] = "File saved. ";
const char msg_load[] = "File loaded. ";
const char msg_new[] = "Memory cleared. ";

// свой PID
Dword myPID = -1;

// начальные размеры
#define WND_W 550
#define WND_H 400
// новые размеры и координаты
int wi = WND_W, he = WND_H;
int win_x, win_y;

// цвета элементов интерфейса
#define GRID_COLOR 0xa0a0a0
#define TEXT_COLOR 0x000000
#define CELL_COLOR 0xffffff
#define SEL_CELL_COLOR 0xe0e0ff
#define FIXED_CELL_COLOR 0xe0e0ff
#define SEL_FIXED_CELL_COLOR 0x758FC1
#define TEXT_SEL_FIXED_COLOR 0xffffff

#define SCROLL_BAR_WIDTH 16
#define SCROLL_BAR_HEIGHT 16

// ID кнопок
#define FILENAME_BUTTON 0x10
#define SAVE_BUTTON 0x11
#define LOAD_BUTTON 0x12
#define NEW_BUTTON 0x13
#define DRAG_BUTTON 0x20

#define SCROLL_LEFT_BUTTON 0x21
#define SCROLL_RIGHT_BUTTON 0x22
#define SCROLL_UP_BUTTON 0x23
#define SCROLL_DOWN_BUTTON 0x24
#define SCROLL_WIDTH 0x25
#define SCROLL_HEIGHT 0x26

#define COL_BUTTON 0x100
#define ROW_BUTTON (COL_BUTTON + 0x100)
#define COL_HEAD_BUTTON (ROW_BUTTON + 0x100)
#define ROW_HEAD_BUTTON (COL_HEAD_BUTTON + 0x100)
#define CELL_BUTTON (ROW_HEAD_BUTTON + 0x100)


// нижняя панель с кнопками и полем ввода
#define MENU_PANEL_HEIGHT 40
Dword panel_y = 0;

// для поля ввода
char edit_text[256] = "";
edit_box cell_box = {0,9*8-5,WND_H - 16-32,0xffffff,0x6a9480,0,0x808080,0,255,(dword)&edit_text,0};

// ячейки - их параметры и текст
DWORD def_col_width = 80, def_row_height = 16;
DWORD col_count = 200, row_count = 100;
DWORD *col_width, *row_height;
char ***cells;
char ***values;	// значения формул, если есть

bool display_formulas = 0;	// отображать ли формулы вместо значений

// координаты отображаемых столбцов и строк
DWORD *col_left, *row_top;

// буфер обмена
char ***buffer = NULL;
DWORD buf_col, buf_row;
DWORD buf_old_x, buf_old_y;

// это координаты ячейки, отображаемой в ЛВ угле
DWORD scroll_x = 1, scroll_y = 1;
// это выделенная ячейка
DWORD sel_x = 1, sel_y = 1;
DWORD prev_x = 0, prev_y = 0;	// предыдущая выделенная
int was_single_selection = 0;

// конец выделения если выделено несколько ячеек
DWORD sel_end_x = sel_x, sel_end_y = sel_y;

// флаг
bool sel_moved = 0;
bool sel_end_move = 0;
// сколько ячеек помещается в окне по х и у
DWORD nx = 0, ny = 0;

// флаг реадктирования ячейки
//bool is_edit = 0;
#define ed_focus 2
#define is_edit (cell_box.flags & ed_focus)

// редактирование имени файла
bool fn_edit = 0;
char fname[256];
edit_box file_box = {0,9*8-5,WND_H - 16-32,0xffffff,0x6a9480,0,0x808080,0,255,(dword)&fname,0};

// изменение размеров
#define SIZE_X 1 // состояние
#define SIZE_Y 2
#define SIZE_SELECT 3
#define SIZE_DRAG 4
int size_mouse_x, size_mouse_y, size_id, size_state = 0;

// растаскивание ячейки при ее тащении за правый нижний угол, с заполнением ячеек
int drag_x, drag_y;
int old_end_x, old_end_y;

void draw_window();

//edit_box ebox = {250,14,35,0xffffff,0x6f9480,0,0xAABBCC,0,248,0,2,20,20};

void kos_DrawRegion(Word x, Word y,Word width, Word height, Dword color1, Word invert)
{
	kos_DrawLine(x,y,x+width-2,y,color1,invert);
	kos_DrawLine(x,y+1,x,y+height-1,color1,invert);
	kos_DrawLine(x+width-1,y,x+width-1,y+height-2,color1,invert);
	kos_DrawLine(x+1,y+height-1,x+width-1,y+height-1,color1,invert);
}

// edit box
/*
void KEdit()
{
	int max_char = (editbox_w) / 6;
	kos_DrawBar(editbox_x,editbox_y,editbox_w-1,editbox_h-3,0xFFFFFF); //белая область
	if (strlen(edit_text)<max_char) 
		kos_WriteTextToWindow(editbox_x, editbox_y+editbox_h / 2-5,0x80,0,edit_text,0); //editbox_h/2+ вместо +3
	else	
		kos_WriteTextToWindow(editbox_x, editbox_y+editbox_h / 2-5,0x80,0,edit_text+strlen(edit_text)-max_char+1,0); //text 'path'
}
*/

void start_edit(int x, int y)
{

	int ch = 0;
	if (x < scroll_x || x > nx - 1)
	{
		scroll_x = x;
		ch = 1;
	}
	if (y < scroll_y || y > ny - 1)
	{
		scroll_y = y;
		ch = 1;
	}
	if (ch)
	{
		sel_moved = 1;
		draw_window();
	}

	file_box.flags &= ~ed_focus;

	cell_box.flags |= ed_focus;
	cell_box.left = col_left[x] + 1;
	cell_box.top = row_top[y] + 1;
	cell_box.width = col_width[x] - 2;
	//cell_box.height= row_height[y];
	memset((Byte*)edit_text, 0, sizeof(edit_text));
	if (cells[x][y])
	{
		strcpy(edit_text, cells[x][y]);
		edit_text[strlen(cells[x][y]) - 1] = '\0';
	}
	cell_box.pos = cell_box.offset = 0;

	draw_window();
}

void stop_edit()
{
	if (is_edit)
	{
		cell_box.flags &= ~ed_focus;
		if (cells[sel_x][sel_y])
			freemem(cells[sel_x][sel_y]);
		if (strlen(edit_text) > 0)
		{
			cells[sel_x][sel_y] = (char*)allocmem(strlen(edit_text)+1);
			strcpy(cells[sel_x][sel_y], edit_text);
		}
		else
			cells[sel_x][sel_y] = NULL;
		//memset((Byte*)edit_text,0, 256);
		calculate_values();
	}
	else
		return;
}

void cancel_edit()
{
	if (!is_edit)
		return;
	cell_box.flags &= ~ed_focus;
	memset((Byte*)edit_text,0, 256);
	draw_window();
}

void check_sel()
{
	DWORD sx0=scroll_x, sy0=scroll_y;

	if (sel_x >= nx - 1  /*&& sel_x < col_count - nx + scroll_x + 1*/)
		//if (sel_x == nx)
			scroll_x++;
		//else
		//	scroll_x = sel_x;
	if (sel_y >= ny - 1 /*&& sel_y < row_count - ny + scroll_y */)
		//if (sel_y == ny)
			scroll_y++;
		//else
		//	scroll_y = sel_y;

	if (sel_x < scroll_x)
		scroll_x = sel_x;
	if (sel_y < scroll_y)
		scroll_y = sel_y;

	if (sx0 != scroll_x || sy0 != scroll_y)
		sel_moved = 0;			// надо перерисовать все

}

// сдвинуть выделение
void move_sel(DWORD new_x, DWORD new_y)
{
	sel_moved = 1;
	stop_edit();
	prev_x = sel_x;
	prev_y = sel_y;
	sel_x = new_x;
	if (sel_x < 1)
		sel_x = 1;
	if (sel_x > col_count - 1)
		sel_x = col_count - 1;
	sel_end_x = sel_x;
	sel_y = new_y;
	if (sel_y < 1)
		sel_y = 1;
	if (sel_y > row_count - 1)
		sel_y = row_count - 1;
	sel_end_y = sel_y;
	check_sel();
	draw_window();
}

void draw_custom_button(int x0, int y0, int sx, int sy, int blue_border)
{
	int x1 = x0 + sx;
	int y1 = y0 + sy;

	if (blue_border) kos_DrawRegion(x0-1, y0-1, sx+3, sy+3, 0x94aece, 0);

	// серый прямоугольник

	kos_DrawBar(x0 + 1, y0 + 1, sx - 1, sy - 1, 0xe4dfe1);
	
	// две белые линии: сверху и слева

	kos_DrawLine(x0, y0, x1, y0, 0xffffff, 0);
	kos_DrawLine(x0, y0, x0, y1, 0xffffff, 0);

	// две серые линии: снизу и справа
	kos_DrawLine(x0, y1, x1, y1, 0xc7c7c7, 0);
	kos_DrawLine(x1, y0, x1, y1, 0xc7c7c7, 0);
}

// x - между low и high ? - необязательно low<high
bool is_between(Dword x, Dword low, Dword high)
{
	return ((low<high)?(x >= low && x <= high):(x >= high && x <= low));
}

void clear_cell_slow(int px, int py)
{
	int i;
	int x0 = col_width[0];
	for (i = scroll_x; i < px; i++)
	{
		x0 += col_width[i];
	}
	int x1 = x0;
	x1 += col_width[px];
	int y0 = row_height[0];
	for (i = scroll_y; i < py; i++)
	{
		y0 += row_height[i];
	}
	int y1 = y0;
	y1 += row_height[py];
	kos_DrawBar(x0 + 1, y0 + 1, x1 - x0 - 1, y1 - y0 - 1, 0xffffff);
}

//debug
const int debugcolor[10]={0xff0000,0x00ff00,0x0000ff,0xffff00,0x00ffff,0xff00ff,0x800000,0x008000,0x000080,0x800080};
int debugc=0;

// рисование ячеек
#define is_x_changed(v) ((v) == sel_x || (v) == prev_x)
#define is_y_changed(v) ((v) == sel_y || (v) == prev_y)

void draw_grid()
{
	int i,j;
	long x0 = 0, y0 = 0, x = 0, y = 0, dx, popravka;
	DWORD text_color;
	//int lx, ly;

//	sprintf(debuf, "%U,%U", scroll_x, scroll_y);
//	rtlDebugOutString(debuf);

	nx=ny=0;

	// очистить область около выделенной ячейки
	if (sel_moved)
	{
		clear_cell_slow(sel_x, sel_y);
		clear_cell_slow(prev_x, prev_y);
	}
	else
	{
		// очистить всю область ячеек
		//kos_DrawBar(col_width[0]+1, row_height[0]+1, wi - SCROLL_BAR_WIDTH-col_width[0]-1, he - SCROLL_BAR_HEIGHT-row_height[0]-1, 0xffffff);
	}

	col_left[0] = 0;
	// ячейки - заголовки столбцов + вертикальные линии
	x = col_width[0]; 
	nx = 1;
	for (i = 1; i < col_count; i++)
	{
		col_left[i] = -1;
		if (i >= scroll_x)
		{
			{				
				if (!sel_moved || is_x_changed(i)) 
					kos_DrawLine(x-x0, 0, x-x0, row_height[0], GRID_COLOR, 0);
			// и заголовок ячейки по х
				text_color = TEXT_COLOR;
				dx = (col_width[i]-6)/2;
				int dy = (row_height[0] - 8) / 2 + 1;
				int cur_width = col_width[i] - 1;
				if (cur_width + x - x0 > wi - SCROLL_BAR_WIDTH)
					cur_width = wi - SCROLL_BAR_WIDTH - x + x0;
				if (!sel_moved || (is_x_changed(i))) 
					if (is_between(i,sel_x,sel_end_x))	
					{
						kos_DrawBar(x - x0 + 1,0,cur_width,row_height[0],SEL_FIXED_CELL_COLOR); //0x0000CC
						text_color = TEXT_SEL_FIXED_COLOR;
					}
					else
					{
						kos_DrawBar(x - x0 + 1,0,cur_width,row_height[0],FIXED_CELL_COLOR);
						text_color = TEXT_COLOR;
					}
				if (!sel_moved || (is_x_changed(i))) kos_WriteTextToWindow(x-x0+2+dx,dy,0,text_color,cells[i][0],strlen(cells[i][0]));

				// есть кнопка стоблца и еще кнопка изменения ширины 
				if (x - x0 + col_width[i] <= wi - col_width[0])
					kos_DefineButton(x-x0+5,0,cur_width - 10,row_height[0]-1,0x60000000+COL_HEAD_BUTTON+i,0);
				//kos_DefineButton(x-x0+col_width[i]-10,0,15,row_height[0]-1,0x60000000+COL_SIZE_BUTTON+i,0);
				col_left[i] = x - x0;
			}
			if (x - x0 > wi - col_width[0])
			{
				x += col_width[i];
				nx++;
				break;
			}
		}
		else
		{
			x0 += col_width[i];
		}
		x += col_width[i];
		nx++;
	}

	//kos_DefineButton(0,0,0,0,0x80000000+COL_HEAD_BUTTON+i,0);

	for (j = i + 1; j < col_count; j++) 
		col_left[j] = wi;
	//if (!sel_moved || (is_x_changed(nx))) kos_DrawLine(x - x0, 0, x - x0, he, GRID_COLOR, 0);

	// ячейки - заголовки строк + горизонт. линии
	y = row_height[0];
	ny = 1;
	row_top[0] = 0;
	for (i = 1; i < row_count && y - y0 < he - 10; i++)
	{
		row_top[i] = -1;
		if (i >= scroll_y)
		{
			{
				if (!sel_moved || (is_y_changed(i))) 
					kos_DrawLine(0, y - y0, wi - SCROLL_BAR_WIDTH, y - y0, GRID_COLOR, 0);
				// и заголовок ячейки по y
				text_color = TEXT_COLOR;
				dx = (col_width[0]-6 * strlen(cells[0][i]))/2;	// optimize this, change strlen
				int dy = (row_height[i] - 8) / 2 + 1;
				if (!sel_moved || (is_y_changed(i)))
					if (is_between(i,sel_y,sel_end_y))
					{
						kos_DrawBar(0,y-y0+1,col_width[0],row_height[i] - 1,SEL_FIXED_CELL_COLOR);
						text_color = TEXT_SEL_FIXED_COLOR;
					}
					else
					{
						kos_DrawBar(0,y-y0+1,col_width[0],row_height[i] - 1,FIXED_CELL_COLOR);
						text_color = TEXT_COLOR;
					}

				if (!sel_moved || (is_y_changed(i)))
					kos_WriteTextToWindow(2+dx,y-y0+dy,0,text_color,cells[0][i],strlen(cells[0][i]));

				kos_DefineButton(0,y-y0+5,col_width[0]-1,row_height[i]-6,0x60000000+ROW_HEAD_BUTTON+i,0);
				//kos_DefineButton(0,y-y0+row_height[i]-5,col_width[0]-1,10,0x60000000+ROW_SIZE_BUTTON+i,0);
				row_top[i] = y - y0;
			}
		}
		else
		{
			y0 += row_height[i];
		}
		y += row_height[i];
		ny++;
	}
	
	kos_DefineButton(0,0,0,0,0x80000000+ROW_HEAD_BUTTON+ny-1,0);

	for (j = i + 1; j < row_count; j++)
		row_top[j] = he;
	if (!sel_moved || (is_y_changed(ny))) 
		kos_DrawLine(0, y - y0, wi - SCROLL_BAR_WIDTH, y - y0, GRID_COLOR, 0);

	if (!sel_moved || (is_x_changed(0) && is_y_changed(0)))
		kos_DrawBar(0,0,col_width[0],row_height[0],FIXED_CELL_COLOR); 
	// ЛВ ячейка

	//sprintf(debuf, "%U, %U; %U, %U", x0, y0, nx, ny);
	//rtlDebugOutString(debuf);

//	popravka = (y - y0 < he - 10);
	//sprintf(debuf, "%U, %U", scroll_y, ny);
	//rtlDebugOutString(debuf);

	
	// сами ячейки

	y = row_height[0];
	for (i = scroll_y; i < ny; i++)
	{
		x = col_width[0];
		if (!sel_moved)
			kos_DrawBar(col_width[0]+1, y+1, wi - SCROLL_BAR_WIDTH-col_width[0]-1, row_height[i]-1, 0xffffff);
		for (j = scroll_x; j < nx-1; j++)
		{
			if (!sel_moved || is_x_changed(j) || is_y_changed(i)) 
				kos_DrawLine(col_left[j], row_top[i], col_left[j], row_height[i], GRID_COLOR, 0);

			// заголовки уже нарисованы - пропускаем их
			if (i && j)	
			{
				//kos_DrawBar(x+1, y+1, col_width[i]-1, row_height[i]-1, 0xffffff);

				//rtlDebugOutString(cap);
				//if (j >= sel_x && j <= sel_end_x && i >= sel_y && i <= sel_end_y)
				if (is_between(j,sel_x,sel_end_x) && is_between(i, sel_y, sel_end_y)	// (j,i) - выделена
					&& ((!sel_moved) || (is_x_changed(j) && is_y_changed(i))))			// и ее нужно нарисовать
				{
					if (i == sel_y && j == sel_x)		// рамка
					{
						kos_DrawBar(x,y,col_width[j],2,TEXT_COLOR);	// up
						kos_DrawBar(x,y,2,row_height[i],TEXT_COLOR);	// left
						kos_DrawBar(x,y+row_height[i]-2,col_width[j]-2-3,2,TEXT_COLOR);				// bottom
						kos_DrawBar(x+col_width[j]-2,y, 2,row_height[i]-2-3,TEXT_COLOR);				// right

						kos_DrawBar(x+col_width[j]-4,y+row_height[i]-4,4,4,TEXT_COLOR);
						//kos_DefineButton(x+col_width[j]-2,y+row_height[i]-2,4,4,0x60000000+DRAG_BUTTON,0x000000);
						drag_x = x + col_width[j] - 4;
						drag_y = y + row_height[i] - 4;
					}
					else
						kos_DrawBar(x + 1,y + 1,col_width[j] - 2,row_height[i] - 2,SEL_CELL_COLOR);	//	выделена но не основная(серая)

				}
				//kos_DefineButton(x,y,col_width[j]-1,row_height[i]-1,0x60000000+CELL_BUTTON+((i << 8) + j),0);

				char *text;
				if (values[j][i] && values[j][i][0] == '#')
				{
					text = cells[j][i];
					kos_DrawRegion(x+1, y+1, col_width[j]-1, row_height[i]-1, 0xff0000, 0);
				}
				else
					text = (values[j][i] && !display_formulas ? values[j][i] : cells[j][i]);

				int dy = (row_height[i] - 8) / 2 + 1;

				if (text)
					if (strlen(text) < col_width[j]/6)
						kos_WriteTextToWindow(x+2,y+dy,0,text_color,text,strlen(text));
					else
						kos_WriteTextToWindow(x+2,y+dy,0,text_color,text,col_width[j]/6);

			}
			if (!sel_moved || is_x_changed(j) || is_y_changed(i))  
				kos_DrawLine(col_left[j]+col_width[j], row_top[i], col_left[j]+col_width[j], row_height[i], GRID_COLOR, 0);
			x += col_width[j];
		} 
		y += row_height[i];
	}

	// Scrolls: 
	// horizontal

	//if (!sel_moved) kos_DrawBar(0, he - SCROLL_BAR_HEIGHT, wi - SCROLL_BAR_WIDTH, SCROLL_BAR_HEIGHT, FIXED_CELL_COLOR);
	//if (!sel_moved) kos_DrawBar(scroll_x * wi / col_count, he - SCROLL_BAR_HEIGHT, wi / col_count, SCROLL_BAR_HEIGHT, SEL_FIXED_CELL_COLOR);
	if (!sel_moved)
	{
		// горизонталь
		kos_DrawBar(17, he - SCROLL_BAR_HEIGHT, wi - SCROLL_BAR_WIDTH - 32, SCROLL_BAR_HEIGHT, 0xced0d0);
		// синие линии
		kos_DrawRegion(0, he - SCROLL_BAR_HEIGHT, wi - SCROLL_BAR_WIDTH, SCROLL_BAR_HEIGHT+1, 0x94aece, 0);
		// левая кнопка
		draw_custom_button(1, he - SCROLL_BAR_HEIGHT + 1, 14, 14, 1);
		kos_WriteTextToWindow(6, he - SCROLL_BAR_HEIGHT + 5, 0, 0, "\x1B", 1);
		// правая
		draw_custom_button(wi - SCROLL_BAR_WIDTH * 2 + 1, he - SCROLL_BAR_HEIGHT + 1, 14, 14, 1);
		kos_WriteTextToWindow(wi - SCROLL_BAR_WIDTH * 2 + 6, he - SCROLL_BAR_HEIGHT + 5, 0, 0, "\x1A", 1);
		// ползунок
		int tmp_w = (nx - scroll_x) * (wi - SCROLL_BAR_WIDTH - 2 * 14 - 14) / (col_count + 1);
		if (tmp_w < 16)
			tmp_w = 16;
		draw_custom_button(17 + (scroll_x - 1) * (wi - SCROLL_BAR_WIDTH - 2 * 14 - 14) / (col_count + 1), he - SCROLL_BAR_HEIGHT + 1, 
			tmp_w, 14, 1);

#define sw(x,y) y,x
// не пинайте меня за это, было лень переставлять руками...

		// вертикаль
		kos_DrawBar(sw(17, wi - SCROLL_BAR_WIDTH), sw(he - SCROLL_BAR_HEIGHT - 33, SCROLL_BAR_WIDTH), 0xced0d0);
		// синие линии
		kos_DrawRegion(sw(0, wi - SCROLL_BAR_WIDTH), sw(he - SCROLL_BAR_HEIGHT, SCROLL_BAR_WIDTH+1), 0x94aece, 0); // up

		// верхняя кнопка
		draw_custom_button(sw(1, wi - SCROLL_BAR_WIDTH + 1), 14, 14, 1);
		kos_WriteTextToWindow(sw(5, wi - SCROLL_BAR_WIDTH + 6), 0, 0, "\x18", 1);
		// нижняя
		draw_custom_button(sw(he - SCROLL_BAR_HEIGHT * 2 + 1, wi - SCROLL_BAR_WIDTH + 1), 14, 14, 1);
		//draw_custom_button(sw(he - SCROLL_BAR_HEIGHT * 2 + 1, wi - SCROLL_BAR_WIDTH + 1), 14, 14, 1);
		kos_WriteTextToWindow(sw(he - SCROLL_BAR_HEIGHT * 2 + 5, wi - SCROLL_BAR_WIDTH + 6), 0, 0, "\x19", 1);
		// ползунок
		int tmp_h = (ny - scroll_y) * (he - SCROLL_BAR_HEIGHT - 2 * 14 - 14) / (row_count + 1);
		if (tmp_h < 16)
			tmp_h = 16;
		draw_custom_button(sw(17 + (scroll_y - 1) * (he - SCROLL_BAR_HEIGHT - 2 * 14 - 14) / (row_count + 1), wi - SCROLL_BAR_WIDTH + 1), 
			sw(tmp_h, 14), 1);
	}
#define NO_DRAW 0x60000000
	kos_DefineButton(1, he - SCROLL_BAR_HEIGHT + 1, 14, 14, NO_DRAW + SCROLL_LEFT_BUTTON,0);
	kos_DefineButton(wi - SCROLL_BAR_WIDTH * 2 + 2, he - SCROLL_BAR_HEIGHT + 1, 14, 14, NO_DRAW + SCROLL_RIGHT_BUTTON,0);
	kos_DefineButton(17, he - SCROLL_BAR_HEIGHT + 1,  (wi - SCROLL_BAR_WIDTH - 2 * 14), 14, NO_DRAW + SCROLL_WIDTH,0);

	kos_DefineButton(sw(1, wi - SCROLL_BAR_WIDTH + 1), 14, 14, NO_DRAW + SCROLL_UP_BUTTON,0);
	kos_DefineButton(sw(he - SCROLL_BAR_HEIGHT * 2 + 2, wi - SCROLL_BAR_WIDTH + 1), 14, 14, NO_DRAW + SCROLL_DOWN_BUTTON,0);
	kos_DefineButton(sw(17, wi - SCROLL_BAR_WIDTH + 1),  sw((he - SCROLL_BAR_HEIGHT - 2 * 14), 14), NO_DRAW + SCROLL_HEIGHT,0);

}

// очень быстрое рисование сетки, в процессе изменения размеров ячеек
void draw_size_grid()
{
	//rtlDebugOutString("draw size grid");

	kos_WindowRedrawStatus(1);

	if (size_state == SIZE_X)
	{
		int x, x0, i;

		x = col_width[0]; 
		x0 = 0;
		for (i = 1; i < col_count && x - x0 + col_width[i] < wi - 10; i++)
		{
			if (i >= scroll_x)
			{
				if (i >= size_id)
					kos_DrawLine(x - x0, 0, x - x0, he, 0, 1);
			}
			else
				x0 += col_width[i];
			x += col_width[i];
		}
		kos_DrawLine(x - x0, 0, x - x0, he, 0, 1);
	}
	else
	{
		int y, y0, i;

		y = row_height[0]; 
		y0 = 0;
		for (i = 1; i < col_count && y - y0 + row_height[i] < he - 10; i++)
		{
			if (i >= scroll_y)
			{
				if (i >= size_id)
					kos_DrawLine(0, y - y0, wi, y - y0, 0, 1);
			}
			else
				y0 += row_height[i];
			y += row_height[i];
		}
		kos_DrawLine(0, y - y0, wi, y - y0, 0, 1);
	}


	kos_WindowRedrawStatus(2);
}


// быстрое рисование выделенной области при выделении мышью
#define DCOLOR 0
//0xff0000
#define DINVERT 1
void draw_drag()
{
	kos_WindowRedrawStatus(1);

	// собственно, 4 инверсные линии

	int k0 = min(sel_x, sel_end_x);
	int k1 = max(sel_x, sel_end_x);
	int n0 = min(sel_y, sel_end_y);
	int n1 = max(sel_y, sel_end_y);

	DWORD x0 = col_left[k0] - 1;
	DWORD x1 = col_left[k1] + col_width[k1] + 1;
	DWORD y0 = row_top[n0] - 1;	
	DWORD y1 = row_top[n1] + row_height[n1] + 1;
	if (x0 > wi - 1) x0 = wi - 1;
	if (x1 > wi - 1) x1 = wi - 1;
	if (y0 > he - 1) y0 = he - 1;
	if (y1 > he - 1) y1 = he - 1;

	//sprintf(debuf,"drag %U %U %U %U",k0,k1,n0,n1);
	//rtlDebugOutString(debuf);

	kos_DrawLine(x0, y0, x0, y1, DCOLOR, DINVERT);
	kos_DrawLine(x0, y0, x1, y0, DCOLOR, DINVERT);
	kos_DrawLine(x1, y0, x1, y1, DCOLOR, DINVERT);
	kos_DrawLine(x0, y1, x1, y1, DCOLOR, DINVERT);

	kos_WindowRedrawStatus(2);
}

void draw_window()
{
	int i;
	double xx0=0.0, yy0=0.0;
	sProcessInfo info;
	void *p;

	if (sel_end_move)
		sel_moved = 0;

	memset((Byte*)&info, 0, 1024);

	kos_ProcessInfo(&info, 0xFFFFFFFF);

	p = info.rawData + 42;			// magic
	wi = *(Dword *)(p);
	he = *(Dword *)((Byte *)p + 4);
	win_x = *(Dword *)((Byte *)p - 8);
	win_y = *(Dword *)((Byte *)p - 4);

	myPID = *(Dword*)((Byte *)p - 12);

	if (wi == 0) 
		wi = WND_W;
	if (he == 0)
		he = WND_H;

	he -= kos_GetSkinHeight() + MENU_PANEL_HEIGHT; // доступная высота окна
	wi -= 6 + 4;

	// start redraw
	kos_WindowRedrawStatus(1);

	kos_DefineAndDrawWindow(10,40,WND_W,WND_H,0x33,0x40FFFFFF,0,0,(Dword)"Table v" TABLE_VERSION);

	if (he + MENU_PANEL_HEIGHT <= 8) 
	{
		kos_WindowRedrawStatus(2); 
		return;
	}

	if (!sel_moved)
	{
		kos_DrawBar(wi-15,he - kos_GetSkinHeight() +7,16,16,0xe4dfe1);
		kos_DrawBar(0,he - kos_GetSkinHeight() + 23,wi + 1,MENU_PANEL_HEIGHT-4,0xe4dfe1);
	}

//	edit_box_draw((dword)&ebox);
	int y = he + kos_GetSkinHeight() - 10;

	if (!sel_moved)
	{
		kos_WriteTextToWindow(3 + 1, y + 3, 0x80 , 0x000000, (char*)sFilename, strlen(sFilename));	
	}

	//DWORD fn_line_color = fn_edit ? 0x000000 : 0xc0c0c0;
	//kos_DrawRegion(61, y - 2, 102, 18, fn_line_color, 0);

	// дальше editbox width = 100

	// border around edit box
	file_box.left = 64;
	file_box.top = y - 1;
	file_box.width = 98;
		//editbox_h = 18;
	//kos_DefineButton(62, y + 3, 100, 16, 0x60000000+FILENAME_BUTTON, 0xd0d0d0);


	// сохранить
	kos_DefineButton(20 + 160, y - 5, 60, 20, SAVE_BUTTON, 0xd0d0d0);
	kos_WriteTextToWindow(22 + 160 + (60 - strlen(sSave) * 6) / 2, y + 2, 0, 0x000000, (char*)sSave, strlen(sSave));

	// загрузить
	kos_DefineButton(90 + 160, y - 5, 60, 20, LOAD_BUTTON, 0xd0d0d0);
	kos_WriteTextToWindow(92 + 160 + (60 - strlen(sLoad) * 6) / 2, y + 2, 0, 0x000000, (char*)sLoad, strlen(sLoad));

	// создать. только эту кнопу воткнуть некуда о_О
	/*
	kos_DefineButton(90 + 160 + 70, y - 5, 60, 20, NEW_BUTTON, 0xd0d0d0);
	kos_WriteTextToWindow(92 + 160 + 10 + 70, y + 2, 0, 0x000000, (char*)sNew, strlen(sNew));
	*/
	panel_y = y;

	draw_grid();
	//kos_DefineButton(0,0,WND_W,WND_H,0x60000002,0);
	//if (is_edit) KEdit();

	if ((void*)edit_box_draw != NULL)
	{
		if (is_edit)
			edit_box_draw((DWORD)&cell_box);
		edit_box_draw((DWORD)&file_box);
	}	

	// end redraw
	kos_WindowRedrawStatus(2);
	sel_moved = 0;
}


void process_mouse()
{
	Dword mouse_btn, ckeys, shift, ctrl;
	int mouse_x, mouse_y, i, p, dx = 0, dy = 0;
	int redraw = 0;
	
	Dword mySlot = kos_GetSlotByPID(myPID);
	if (kos_GetActiveSlot() != mySlot)
		return;

	edit_box_mouse((dword)&cell_box);
	edit_box_mouse((dword)&file_box);

	int vert, hor;
	kos_GetScrollInfo(vert, hor);

	//sprintf(debuf, "scroll %U %U", vert, hor);
	//rtlDebugOutString(debuf);

	if (vert != 0) //труъ перерисовка!
	{
		move_sel(sel_x, sel_y + vert);
		//move_sel(sel_x + hor, sel_y);
		return;
	}
	
	kos_GetMouseState(mouse_btn, mouse_x, mouse_y);
	mouse_x -= 5;
	mouse_y -= kos_GetSkinHeight();
	mouse_btn &= 0x0001;

	ckeys = kos_GetSpecialKeyState();
	shift = ckeys & 0x3;

	if (!size_state && !mouse_btn)
		return;
	if (mouse_btn && !size_state)		// LMB down				
	{
		//rtlDebugOutString("lmb down and not resize");

		if (mouse_x >= drag_x && mouse_x <= drag_x + 4 && mouse_y >= drag_y && mouse_y <= drag_y + 4)
		{
			size_state = SIZE_DRAG;
			old_end_x = sel_end_x;
			old_end_y = sel_end_y;
		}
		else if (mouse_y <= row_height[0])
		{
			//rtlDebugOutString("can resize cols");
			int kx = -1, i;
			for (i = 0; i < col_count - 1; i++)
			if (mouse_x >= col_left[i] + col_width[i] - 5 &&
				mouse_x <= col_left[i + 1] + 5)
			{
				kx = i; break;
			}
			if (kx != -1)
			{
				//sprintf(debuf,"size x %U",k);
				//rtlDebugOutString(debuf);
				size_id = kx;
				size_state = SIZE_X;
			}
		}
		else if (mouse_x <= col_width[0])
		{
			int ky = -1;
			for (i = 0; i < row_count - 1; i++)
			if (mouse_y >= row_top[i] + row_height[i] - 5 &&
				mouse_y <= row_top[i + 1] + 5)
			{
				ky = i; break;
			}
			if (ky != -1)
			{
				size_id = ky;
				size_state = SIZE_Y;
			}
		}
		else		// кликнута ячейка
		if (mouse_x <= col_left[nx - 1] &&  mouse_y <= row_top[ny - 1])
		{
			was_single_selection = sel_x == sel_end_x && sel_y == sel_end_y;
			int kx = -1, i;
			for (i = 0; i < col_count - 1; i++)
			if (mouse_x >= col_left[i] &&
				mouse_x <= col_left[i] + col_width[i])
			{
				kx = i; break;
			}
			int ky = -1;
			for (i = 0; i < row_count - 1; i++)
			if (mouse_y >= row_top[i] &&
				mouse_y <= row_top[i] + row_height[i])
			{
				ky = i; break;
			}
			if (kx != -1 && ky != -1)
			{
				if (!shift) 
				{
					move_sel(kx, ky);
					return;
				}
				else
				{
					sel_end_x = kx;
					sel_end_y = ky;
				}
				size_state = SIZE_SELECT;
			}
		}
		if (size_state)
		{
			size_mouse_x = mouse_x;
			size_mouse_y = mouse_y;
		}
		return;
	}
	else if (!mouse_btn && size_state)
	{
		sel_moved = 0;		// чтобы была тру перерисовка
		//rtlDebugOutString("resize end");

		if (size_state == SIZE_DRAG)
		{
			fill_cells(sel_x, sel_y, sel_end_x, sel_end_y, old_end_x, old_end_y);
		}

		sel_moved = (size_state == SIZE_SELECT && sel_x == sel_end_x && sel_y == sel_end_y && was_single_selection);
		size_state = 0;
		draw_window();		// все сдвинулось - надо обновиться
		return;
	}
	if (size_state == SIZE_X && mouse_x != size_mouse_x)
	{
		draw_size_grid();
		col_width[size_id] += mouse_x - size_mouse_x;
		if (col_width[size_id] < 15)
			col_width[size_id] = 15;
		else if (col_width[size_id] > wi / 2)
			col_width[size_id] = wi / 2;
		draw_size_grid();
	}
	if (size_state == SIZE_Y && mouse_y != size_mouse_y)
	{
		draw_size_grid();
		row_height[size_id] += mouse_y - size_mouse_y;
		if (row_height[size_id] < 15)
			row_height[size_id] = 15;
		else if (row_height[size_id] > he / 2)
			row_height[size_id] = he / 2;
		draw_size_grid();
	}
	if ((size_state == SIZE_SELECT || size_state == SIZE_DRAG) && (mouse_x != size_mouse_x || mouse_y != size_mouse_y))
	{
		draw_drag();
		int kx = -1, i;
		for (i = 0; i < col_count - 1; i++)
			if (mouse_x >= col_left[i] &&
				mouse_x <= col_left[i + 1])
			{
				sprintf(debuf, "yyy %U",col_left[i+1]);
				rtlDebugOutString(debuf);
				kx = i; break;
			}
		int ky = -1;
		for (i = 0; i < row_count - 1; i++)
			if (mouse_y >= row_top[i] &&
				mouse_y <= row_top[i + 1])
			{
				ky = i; break;
			}
		if (kx != -1) sel_end_x = kx;
		if (kx != -1) sel_end_y = ky;
		if (size_state == SIZE_DRAG)
		{
			if (abs(sel_end_x - sel_x) > 0)
			{
				sel_end_y = old_end_y;
			}
			else if (abs(sel_end_y - sel_y) > 0)
			{
				sel_end_x = old_end_x;
			}
		}
		draw_drag();
	}
	size_mouse_x = mouse_x;
	size_mouse_y = mouse_y;
}

void process_key()
{
	Dword mouse_btn, ckeys, shift, ctrl;
	int mouse_x, mouse_y, i, p, dx = 0, dy = 0;

	// key pressed, read it 
	Byte keyCode;
	ckeys = kos_GetSpecialKeyState();
	shift = ckeys & 0x3;
	ctrl = ckeys & 0x0c;
	//if (ctrl)
	//	rtlDebugOutString("control pressed!");
	dx = 0, dy = 0;
	sel_moved = 0;
	sel_end_move = 0;
	kos_GetKey(keyCode);

	__asm
	{
		mov ah, keyCode
	}
	edit_box_key((dword)&cell_box);
	edit_box_key((dword)&file_box);


	switch (keyCode)
	{
		case 178:			// стрелки
			//dx = 0;
			dy = -1;
			break;
		case 176:
			dx = -1;
			//dy = 0;
			break;
		case 179:
			dx = 1;
			//dy = 0;
			break;
		case 177:
			//dx = 0;
			dy = 1;
			break;
		/*
		case 183:
			if (sel_y < row_count-(ny - scroll_y))	// page down
				dy = ny - scroll_y;
			else
				dy = row_count-(ny - scroll_y) - sel_y;
			dx = 0;
			redraw = 1;
			break;
		case 184:
			if (sel_y > ny - scroll_y)		// page up
				dy= - (ny - scroll_y);
			else
				dy = - (ny - scroll_y) + sel_y;
			dx = 0;
			redraw = 1;
			break;
		*/
		case 180: //home
			dx = -sel_x + 1;
			dy = 0;
			draw_grid(); //draw_window(); 
			break;
		case 181: //end
			dx = col_count - (nx - scroll_x) - 1 - sel_x;
			dy = 0;
			draw_grid(); //draw_window();
			break;
		case 27:		// escape
			cancel_edit();
			break;
		case 182:			// delete
			{
				int i,j,n0,n1,k0,k1;
				n0 = min(sel_x, sel_end_x);
				n1 = max(sel_x, sel_end_x);
				k0 = min(sel_y, sel_end_y);
				k1 = max(sel_y, sel_end_y);

				for (i = n0; i <= n1; i++)
					for (j = k0; j <= k1; j++)
					{
						if (cells[i][j])
						{
							freemem(cells[i][j]);
							cells[i][j] = NULL;
						}
					}
				calculate_values();
				draw_grid();
				break;
			}
		case 0x0D:			// enter
			if (is_edit)
			{
				stop_edit();
				draw_window();
			}
			break;
		//case 0x08:			// backspace
			/*if (is_edit || fn_edit)
			{
				if (strlen(edit_text) != 0)
					edit_text[strlen(edit_text) - 1] = '\0';
				KEdit();
			}
			else if (cells[sel_x][sel_y])
			{
				start_edit(sel_x, sel_y);
			}
			*/
		//	break;
		case 22:	// contol-v
			{
				if (ctrl)
				{
					//rtlDebugOutString("control-v!");
					int i, j, x0, y0;
					x0 = min(sel_x, sel_end_x);
					y0 = min(sel_y, sel_end_y);
					int delta_x = x0 - buf_old_x;
					int delta_y = y0 - buf_old_y;

					for (i = 0; i < buf_col; i++)
						for (j = 0; j < buf_row; j++)
						{
							if (i + x0 >= col_count || j + y0 >= row_count)
								continue;
							if (cells[i + x0][j + y0])
								freemem(cells[i + x0][j + y0]);
							if (buffer[i][j])
							{
								cf_x0 = buf_old_x; cf_y0 = buf_old_y;
								cf_x1 = buf_old_x + buf_col;
								cf_y1 = buf_old_y + buf_row;
								cells[i + x0][j + y0] = change_formula(buffer[i][j], delta_x, delta_y);
								//cells[i + x0][j + y0] = (char*)allocmem(strlen(buffer[i][j]));
								//strcpy(cells[i + x0][j + y0], buffer[i][j]);
							}
							else
								cells[i + x0][j + y0] = NULL;
						}

					calculate_values();
					draw_window();
					break;
				}
			}
			case 24:	// control-x
			case 03:	// control-c
			{
				if (ctrl)
				{
					//rtlDebugOutString("control-c!");
					int i, j, x0, y0;

					freeBuffer();

					buf_col = abs(sel_end_x - sel_x) + 1;
					buf_row = abs(sel_end_y - sel_y) + 1;
					x0 = min(sel_x, sel_end_x);
					y0 = min(sel_y, sel_end_y);
					buf_old_x = x0;
					buf_old_y = y0;

					//sprintf(debuf, "%U %U %U %U", buf_col, buf_row, x0, y0);
					//rtlDebugOutString(debuf);
				
					buffer = (char***)allocmem(buf_col * sizeof(char**));
					for (i = 0; i < buf_col; i++)
					{
						buffer[i] = (char**)allocmem(buf_row * sizeof(char*));
						for (j = 0; j < buf_row; j++)
						{
							if (cells[i + x0][j + y0])
							{
								if (keyCode == 03)	// ctrl-c
								{
									buffer[i][j] = (char*)allocmem(strlen(cells[i + x0][j + y0]));
									strcpy(buffer[i][j], cells[i + x0][j + y0]);
								}
								else
								{
									buffer[i][j] = cells[i + x0][j + y0];
									cells[i + x0][j + y0] = NULL;
								}
							}
							else
								buffer[i][j] = NULL;
						}
					}
					if (keyCode == 24) 
						calculate_values();
					draw_window();
					break;
				}
			}
		case 06:		// control-f
			{
				display_formulas = !display_formulas;
				draw_grid(); //draw_window();
				break;
			}
		default:
			
			if (!is_edit && !(file_box.flags & ed_focus))
			{
				start_edit(sel_x, sel_y);
				if (keyCode == 8)
				{
					cell_box.pos = strlen(edit_text);
				}
				else
				{
					__asm
					{
						mov ah, keyCode
					}
					edit_box_key((dword)&cell_box);
				}
			}
			if (is_edit)
				edit_box_draw((dword)&cell_box);
			/*
			if (strlen(edit_text)<256)
			{
				edit_text[strlen(edit_text)]=keyCode;
				edit_text[strlen(edit_text) + 1]='\0';
				KEdit();
			}
			*/
			break;
	}
	if (dx != 0)
	{
		if (shift)
		{
			sel_end_x += dx;
			if (sel_end_x <= 1)
				sel_end_x = 1;
			else if (sel_end_x >= col_count)
				sel_end_x = col_count - 1;
		//	sprintf(debuf,"sel end x change. sel end %U %U",sel_end_x,sel_end_y);
		//	rtlDebugOutString(debuf);
			sel_moved = sel_end_move = 1;
			//stop_edit();
			//draw_grid();
		}
		else
		{
		}
	}
	if (dy != 0)
	{
		if (shift)
		{
			sel_end_y += dy;
			if (sel_end_y <= 1)
				sel_end_y = 1;
			else if (sel_end_y >= row_count)
				sel_end_y = row_count - 1;
		//	sprintf(debuf,"sel end y change. sel end %U %U",sel_end_x,sel_end_y);
		//	rtlDebugOutString(debuf);
			sel_moved = sel_end_move = 1;
			//stop_edit();
			//draw_grid();
		}
		else
		{
		}
	}
	/*
	if (sel_end_x < sel_x)
	{
		Dword tmp = sel_end_x; sel_end_x = sel_x; sel_x = tmp;
	}
	if (sel_end_y < sel_y)
	{
		Dword tmp = sel_end_y; sel_end_y = sel_y; sel_y = tmp;
	}
	*/
	if ((dx || dy))
	{
		if (!shift)
		{
			move_sel(sel_x + dx, sel_y + dy);
		}
		else
		{
			sel_moved = 0;
			stop_edit();
			draw_grid();
		}
	}
}


void process_button()
{
	Dword mouse_btn, ckeys, shift, ctrl;
	int mouse_x, mouse_y, i, p, dx = 0, dy = 0;
	int redraw = 0;

	Dword button;
	kos_GetButtonID(button);

	/*
	sprintf(debuf, "button %U", button);
	rtlDebugOutString(debuf);
	//*/

	switch (button)
	{
	case 1:
		kos_ExitApp();

	case SCROLL_LEFT_BUTTON:
		//rtlDebugOutString("scroll left btn");
		stop_edit();
		scroll_x--;
		if (scroll_x <= 0)
			scroll_x = 1;
		sel_moved = 0;
		/*if (sel_x > nx - 1)
		{
			nx - 1;
			sel_end_x = sel_x;
		}*/
		draw_window();
		return;

	case SCROLL_RIGHT_BUTTON:
		//rtlDebugOutString("scroll right btn");
		stop_edit();
		scroll_x++;
		if (scroll_x >= col_count - 1)
			scroll_x = col_count - 1;
		sel_moved = 0;/*
		if (sel_x < scroll_x)
		{
			sel_x = scroll_x;
			sel_end_x = sel_x;
		}*/
		draw_window();
		return;

	case SCROLL_WIDTH:
		{
			//rtlDebugOutString("scroll width btn");
			stop_edit();
			kos_GetMouseState(mouse_btn, mouse_x, mouse_y);
			mouse_x -= 5;
			mouse_y -= kos_GetSkinHeight();

			// всего: wi - SCROLL_BAR_WIDTH - 2 * 14

			int tmp_w = (nx - scroll_x) * (wi - SCROLL_BAR_WIDTH - 3 * 14) / (col_count + 1);
			if (tmp_w < 16)
				tmp_w = 16;
			scroll_x = (mouse_x - 14 - tmp_w / 2) * (col_count + 1) / (wi - SCROLL_BAR_WIDTH - 3 * 14) + 1;
			if (scroll_x <= 0)
				scroll_x = 1;
			else if (scroll_x >= col_count - 1)
				scroll_x = col_count - 1;
			sel_moved = 0;
			draw_window();
			return;
		}

	case SCROLL_UP_BUTTON:
		stop_edit();
		scroll_y--;
		if (scroll_y <= 0)
			scroll_y = 1;
		sel_moved = 0;
		//draw_window();
		draw_grid();
		/*
		if (sel_y > ny - 1)
		{
			sel_y = ny - 1;
			sel_end_y = sel_y;
		}*/
		return;

	case SCROLL_DOWN_BUTTON:
		stop_edit();
		scroll_y++;
		if (scroll_y >= row_count - 1)
			scroll_y = row_count - 1;
		sel_moved = 0;/*
		if (sel_y < scroll_y)
		{
			sel_y = scroll_y;
			sel_end_y = sel_y;
		}*/
		draw_grid();
		return;

	case SCROLL_HEIGHT:
		{
			stop_edit();
			kos_GetMouseState(mouse_btn, mouse_x, mouse_y);
			mouse_x -= 5;
			mouse_y -= kos_GetSkinHeight();
			int tmp_h = (ny - scroll_y) * (he - SCROLL_BAR_HEIGHT - 2 * 14) / row_count;
			if (tmp_h < 16)
				tmp_h = 16;
			scroll_y = (mouse_y - 2 * 14) * (row_count + 1) / (he - SCROLL_BAR_HEIGHT - 3 * 14) + 1;
			if (scroll_y <= 0)
				scroll_y = 1;
			else if (scroll_y >= row_count - 1)
				scroll_y = row_count - 1;
			sel_moved = 0;
			draw_grid();
			return;
		}

	case NEW_BUTTON:	// clear the table
		reinit();
		draw_window();
		break;

	case FILENAME_BUTTON:
		sel_moved = 1;
		stop_edit();
		fn_edit = 1;
		strcpy(edit_text, fname);
		draw_window();
		break;

	case SAVE_BUTTON:
		stop_edit();
		kos_DrawBar(320, panel_y, wi - 320 - 10, 10, 0xe4dfe1);
		if (SaveFile(fname))
			kos_WriteTextToWindow(320, panel_y, 0, 0x000000, (char*)msg_save, strlen(msg_save));
		break;

	case LOAD_BUTTON:
		stop_edit();
		int r = LoadFile(fname);
		kos_DrawBar(320, panel_y, wi - 320 - 10, 10, 0xe4dfe1);
		if (r > 0)
		{
			calculate_values();
			sel_moved = 0;
			draw_window();
			kos_WriteTextToWindow(320, panel_y,0,0x000000,(char*)msg_load, strlen(msg_load));
		}
		else if (r == -1)
			kos_WriteTextToWindow(320, panel_y,0,0x000000,(char*)er_file_not_found,strlen(er_file_not_found));
		else if (r == -2)
			kos_WriteTextToWindow(320, panel_y,0,0x000000,(char*)er_format,strlen(er_format));
		break;
	}
	/*
	if (button >= COL_BUTTON && button < ROW_BUTTON)
	{
		scroll_x = button - COL_BUTTON;
		fn_edit = is_edit = 0;
		draw_window();
		//sprintf(debuf, "col %U", scroll_x);
		//rtlDebugOutString(debuf);
	}
	else if (button >= ROW_BUTTON && button < COL_HEAD_BUTTON)
	{
		scroll_y = button - ROW_BUTTON;
		fn_edit = is_edit = 0;
		draw_window();
		//sprintf(debuf, "row %U", scroll_y);
		//rtlDebugOutString(debuf);
	}
	*/
	if (button >= COL_HEAD_BUTTON && button < ROW_HEAD_BUTTON)
	{
		sel_end_x = sel_x = button - COL_HEAD_BUTTON;
		sel_y = 1;
		sel_end_y = row_count - 1;
		stop_edit();
		draw_window();
		return;
	}
	else if (button >= ROW_HEAD_BUTTON && button < CELL_BUTTON)
	{
		sel_end_y = sel_y = button - ROW_HEAD_BUTTON;
		sel_x = 1;
		sel_end_x = col_count - 1;
		stop_edit();
		draw_window();
		return;
	}

}

void kos_Main()
{
	kos_InitHeap();
	load_edit_box();

	init();
	draw_window();
	for (;;)
	{
		switch (kos_WaitForEvent(10))	// да, плохо. потом нужно будет просто ловить события мыши.
		{
		case 0:
			process_mouse();
			break;
		case 1:
			draw_window();
			break;
		case 2:
			process_key();
			break;
		case 3:
			process_button();
			break;
		//case 6:
		//	draw_window();
		//	break;
		}
	}
}


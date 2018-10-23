//не идёт дальше 98 строки 
//если выделить область ячеек и сдвинуть курсор ввода с помощью клавиш, "следы" остануться
//нельзя перемещаться по буквам в редактируемой строке

#include "func.h"
#include "parser.h"
#include "calc.h"
#include "use_library.h"

#define TABLE_VERSION "0.98.1"

// строки, которые выводит программа
const char *sFileSign = "KolibriTable File\n";
const char sFilename[] = "Filename:";
const char sSave[] = "Save";
const char sLoad[] = "Load";
const char sNew[] = "New";

const char er_file_not_found[] = "Cannot open file. ";
const char er_format[] = "Error: bad format. ";
const char msg_save[] = "File saved. ";
const char msg_load[] = "File loaded. ";
const char msg_new[] = "Memory cleared. ";

// initial window sizes
#define WND_W 640
#define WND_H 480
// new window size and coordinates
int cWidth;
int cHeight;

// interface colors
#define GRID_COLOR 0xa0a0a0
#define TEXT_COLOR 0x000000
#define CELL_COLOR 0xffffff
#define SEL_CELL_COLOR 0xe0e0ff
#define FIXED_CELL_COLOR 0xe0e0ff
#define SEL_FIXED_CELL_COLOR 0x758FC1
#define TEXT_SEL_FIXED_COLOR 0xffffff
#define PANEL_BG_COLOR 0xe4dfe1

#define SCROLL_SIZE 16

// button IDs
#define FILENAME_BUTTON 0x10
#define SAVE_BUTTON 0x11
#define LOAD_BUTTON 0x12
#define NEW_BUTTON 0x13
#define DRAG_BUTTON 0x20

#define COL_BUTTON 0x100
#define ROW_BUTTON (COL_BUTTON + 0x100)
#define COL_HEAD_BUTTON (ROW_BUTTON + 0x100)
#define ROW_HEAD_BUTTON (COL_HEAD_BUTTON + 0x100)
#define CELL_BUTTON (ROW_HEAD_BUTTON + 0x100)

// bottom panel
#define MENU_PANEL_HEIGHT 40
Dword panel_y = 0;
Dword mouse_dd;

// editbox data
char edit_text[256] = "";
edit_box cell_box = {0,9*8-5,WND_H - 16-32,0xffffff,0x6a9480,0,0x808080,0,255,(dword)&edit_text,(dword)&mouse_dd,0};
scroll_bar scroll_v = { SCROLL_SIZE,200,398, NULL, SCROLL_SIZE,0,115,15,0,0xeeeeee,0xD2CED0,0x555555,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};
scroll_bar scroll_h = { 200,NULL,SCROLL_SIZE, NULL, SCROLL_SIZE,0,115,15,0,0xeeeeee,0xD2CED0,0x555555,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

// ячейки - их параметры и текст
DWORD def_col_width = 80, def_row_height = 16;
DWORD col_count = 200, row_count = 100;
DWORD *col_width, *row_height;
char ***cells;

struct GRID
{
	int x,y,w,h;
} grid;

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
edit_box file_box = {98,9*8-5,WND_H - 16-32,0xffffff,0x6a9480,0,0x808080,0,255,(dword)&fname,(dword)&mouse_dd,0};

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

void kos_DrawRegion(Word x, Word y,Word width, Word height, Dword color1, Word invert)
{
	kos_DrawLine(x,y,x+width-2,y,color1,invert);
	kos_DrawLine(x,y+1,x,y+height-1,color1,invert);
	kos_DrawLine(x+width-1,y,x+width-1,y+height-2,color1,invert);
	kos_DrawLine(x+1,y+height-1,x+width-1,y+height-1,color1,invert);
}

void kos_DebugValue(char *str, int n)
{
	char debuf[50];
	sprintf(debuf, "%S: %U\n", str, n);
	rtlDebugOutString(debuf);
}

void DrawScrolls()
{
	// HOR
	scroll_h.x = 0;
	scroll_h.y = grid.y + grid.h;
	scroll_h.w = grid.w;
	scroll_h.all_redraw = true;
	scroll_h.max_area = col_count;
	scroll_h.cur_area = nx-scroll_x-1;
	scroll_h.position = scroll_x-1;
	scrollbar_h_draw((DWORD)&scroll_h);

	// VER
	scroll_v.x = grid.x + grid.w;
	scroll_v.y = 0;
	scroll_v.h = grid.h;
	scroll_v.all_redraw = true;
	scroll_v.max_area = row_count;
	scroll_v.cur_area = ny-scroll_y-1;
	scroll_v.position = scroll_y-1;
	scrollbar_v_draw((DWORD)&scroll_v);
}

void DrawSelectedFrame(int x, int y, int w, int h, DWORD col)
{
	kos_DrawBar(x,y,w,2,col);          // up
	kos_DrawBar(x,y,2,h,col);          // left
	kos_DrawBar(x,y+h-2,w-2-3,2,col);  // bottom
	kos_DrawBar(x+w-2,y, 2,h-2-3,col); // right
	kos_DrawBar(x+w-4,y+h-4,4,4,col);
}

void kos_DeleteButton(int id)
{
	kos_DefineButton(NULL, NULL, NULL, NULL, id+BT_DEL, NULL);
}

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

void move_selection(DWORD new_x, DWORD new_y)
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



// рисование ячеек
#define is_x_changed(v) ((v) == sel_x || (v) == prev_x)
#define is_y_changed(v) ((v) == sel_y || (v) == prev_y)

void draw_grid()
{
	int i,j;
	long x0 = 0, y0 = 0, x = 0, y = 0, dx;
	DWORD text_color;
	DWORD bg_color;

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
		//kos_DrawBar(col_width[0]+1, row_height[0]+1, grid.w - SCROLL_SIZE-col_width[0]-1, he - SCROLL_SIZE-row_height[0]-1, 0xffffff);
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
				if (!sel_moved || is_x_changed(i)) {
					kos_DrawLine(x-x0, 0, x-x0, row_height[0], GRID_COLOR, 0);
				}
				// и заголовок ячейки по х
				text_color = TEXT_COLOR;
				dx = (col_width[i]-6)/2;
				int dy = (row_height[0] - 8) / 2 + 1;
				int cur_width = col_width[i] - 1;
				if (cur_width + x - x0 > grid.w)
					cur_width = grid.w - x + x0 -1;
				if (!sel_moved || (is_x_changed(i))) {
					if (is_between(i,sel_x,sel_end_x))	
					{
						bg_color = SEL_FIXED_CELL_COLOR; 
						text_color = TEXT_SEL_FIXED_COLOR;
					}
					else
					{
						bg_color = FIXED_CELL_COLOR;
						text_color = TEXT_COLOR;
					}
					kos_DrawBar(x - x0 + 1,0,cur_width,row_height[0],bg_color);
					kos_WriteTextToWindow(x-x0+2+dx,dy,0,text_color,cells[i][0],strlen(cells[i][0]));	
				}
				// есть кнопка стоблца и еще кнопка изменения ширины 
				if (x - x0 + col_width[i] <= grid.w - col_width[0])
				{
					kos_DeleteButton(COL_HEAD_BUTTON+i);
					kos_DefineButton(x-x0+5,0,cur_width - 10,row_height[0]-1,BT_NODRAW+COL_HEAD_BUTTON+i,0);
				}
				//kos_DefineButton(x-x0+col_width[i]-10,0,15,row_height[0]-1,BT_NODRAW+COL_SIZE_BUTTON+i,0);
				col_left[i] = x - x0;
			}
			if (x - x0 > grid.w - col_width[0])
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
		col_left[j] = grid.w;
	//if (!sel_moved || (is_x_changed(nx))) kos_DrawLine(x - x0, 0, x - x0, grid.h, GRID_COLOR, 0);

	// ячейки - заголовки строк + горизонт. линии
	y = row_height[0];
	ny = 1;
	row_top[0] = 0;
	for (i = 1; i < row_count && y - y0 < grid.h; i++)
	{
		row_top[i] = -1;
		if (i >= scroll_y)
		{
			{
				if (!sel_moved || (is_y_changed(i))) 
					kos_DrawLine(0, y - y0, grid.w - 1, y - y0, GRID_COLOR, 0);
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

				kos_DeleteButton(ROW_HEAD_BUTTON+i);
				kos_DefineButton(0,y-y0+5,col_width[0]-1,row_height[i]-6,BT_NODRAW+ROW_HEAD_BUTTON+i,0);
				//kos_DefineButton(0,y-y0+row_height[i]-5,col_width[0]-1,10,BT_NODRAW+ROW_SIZE_BUTTON+i,0);
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
		row_top[j] = grid.h;
	if (!sel_moved || (is_y_changed(ny))) 
		kos_DrawLine(0, y - y0, grid.w, y - y0, GRID_COLOR, 0);

	if (!sel_moved || (is_x_changed(0) && is_y_changed(0)))
		kos_DrawBar(0,0,col_width[0],row_height[0],FIXED_CELL_COLOR); 
	// ЛВ ячейка

	//sprintf(debuf, "%U, %U; %U, %U", x0, y0, nx, ny);
	//rtlDebugOutString(debuf);

	// cells itself

	y = row_height[0];
	for (i = scroll_y; i < ny; i++)
	{
		x = col_width[0];
		if (!sel_moved)
			kos_DrawBar(col_width[0]+1, y+1, grid.w -col_width[0]-1, row_height[i]-1, 0xffffff);
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
				if (is_between(j,sel_x,sel_end_x) && is_between(i, sel_y, sel_end_y)	// (j,i) - âûäåëåíà
				&& ((!sel_moved) || (is_x_changed(j) && is_y_changed(i))))			// è åå íóæíî íàðèñîâàòü
				{
					if (i == sel_y && j == sel_x) // frame
					{
						DrawSelectedFrame(x+1,y, col_width[j]-1, row_height[j], TEXT_COLOR);
						drag_x = x + col_width[j] - 4;
						drag_y = y + row_height[i] - 4;
					}
					else
						kos_DrawBar(x + 1,y + 1,col_width[j] - 2,row_height[i] - 2,SEL_CELL_COLOR);	//	âûäåëåíà íî íå îñíîâíàÿ(ñåðàÿ)

				}
				//kos_DefineButton(x,y,col_width[j]-1,row_height[i]-1,BT_NODRAW+CELL_BUTTON+((i << 8) + j),0);

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

	DrawScrolls();
}

// очень быстрое рисование сетки, в процессе изменения размеров ячеек
void draw_size_grid()
{
	//rtlDebugOutString("draw size grid");

	if (size_state == SIZE_X)
	{
		int x, x0, i;

		x = col_width[0]; 
		x0 = 0;
		for (i = 1; i < col_count && x - x0 + col_width[i] < grid.w - 10; i++)
		{
			if (i >= scroll_x)
			{
				if (i >= size_id)
					kos_DrawLine(x - x0, 0, x - x0, grid.h, 0, 1);
			}
			else
				x0 += col_width[i];
			x += col_width[i];
		}
		kos_DrawLine(x - x0, 0, x - x0, grid.h, 0, 1);
	}
	else
	{
		int y, y0, i;

		y = row_height[0]; 
		y0 = 0;
		for (i = 1; i < col_count && y - y0 + row_height[i] < grid.h - 10; i++)
		{
			if (i >= scroll_y)
			{
				if (i >= size_id)
					kos_DrawLine(0, y - y0, grid.w, y - y0, 0, 1);
			}
			else
				y0 += row_height[i];
			y += row_height[i];
		}
		kos_DrawLine(0, y - y0, grid.w, y - y0, 0, 1);
	}

}


// быстрое рисование выделенной области при выделении мышью
#define DCOLOR 0
//0xff0000
#define DINVERT 1
void draw_drag()
{
	// inverted lines

	int k0 = min(sel_x, sel_end_x);
	int k1 = max(sel_x, sel_end_x);
	int n0 = min(sel_y, sel_end_y);
	int n1 = max(sel_y, sel_end_y);

	DWORD x0 = col_left[k0] - 1;
	DWORD x1 = col_left[k1] + col_width[k1] + 1;
	DWORD y0 = row_top[n0] - 1;	
	DWORD y1 = row_top[n1] + row_height[n1] + 1;
	if (x0 > grid.w - 1) x0 = grid.w - 1;
	if (x1 > grid.w - 1) x1 = grid.w - 1;
	if (y0 > grid.h - 1) y0 = grid.h - 1;
	if (y1 > grid.h - 1) y1 = grid.h - 1;

	//sprintf(debuf,"drag %U %U %U %U",k0,k1,n0,n1);
	//rtlDebugOutString(debuf);

	kos_DrawLine(x0, y0, x0, y1, DCOLOR, DINVERT);
	kos_DrawLine(x0, y0, x1, y0, DCOLOR, DINVERT);
	kos_DrawLine(x1, y0, x1, y1, DCOLOR, DINVERT);
	kos_DrawLine(x0, y1, x1, y1, DCOLOR, DINVERT);
}

bool draw_and_define_window()
{
	kos_WindowRedrawStatus(1);
	kos_DefineAndDrawWindow(110,40,WND_W,WND_H,0x73,0x40FFFFFF,0,0,(Dword)"Table v" TABLE_VERSION);
	kos_WindowRedrawStatus(2); 

	sProcessInfo info;
	kos_ProcessInfo(&info, 0xFFFFFFFF);
	cWidth = info.processInfo.width - 9;
	cHeight = info.processInfo.height - kos_GetSkinHeight() - 4;

	grid.x = 0;
	grid.y = 0;
	grid.w = cWidth - SCROLL_SIZE;
	grid.h = cHeight - MENU_PANEL_HEIGHT - SCROLL_SIZE;

	if (info.processInfo.status_window&0x04) return false; //draw nothing if window is rolled-up

	if (grid.h < 100) { kos_ChangeWindow( -1, -1, -1, 180 ); return false; }
	if (grid.w < 340) { kos_ChangeWindow( -1, -1, 350, -1 ); return false; }

	return true;
}

void draw_window()
{

	if (sel_end_move)
		sel_moved = 0;

	panel_y = cHeight - MENU_PANEL_HEIGHT;

	if (!sel_moved)
	{
		kos_DrawBar(cWidth-SCROLL_SIZE, panel_y - SCROLL_SIZE, SCROLL_SIZE, SCROLL_SIZE, PANEL_BG_COLOR);
		kos_DrawBar(0, panel_y, cWidth, MENU_PANEL_HEIGHT, PANEL_BG_COLOR);
		kos_WriteTextToWindow(3 + 1, panel_y + 16, 0x80, 0x000000, (char*)sFilename, 0);	
	}

	file_box.top = panel_y + 12;

	//save
	kos_DefineButton(20 + 160, panel_y + 9, 60, 20, SAVE_BUTTON, 0xd0d0d0);
	kos_WriteTextToWindow(22 + 160 + (60 - strlen(sSave) * 6) / 2, panel_y + 16, 0x80, 0x000000, (char*)sSave, 0);

	//load
	kos_DefineButton(90 + 160, panel_y + 9, 60, 20, LOAD_BUTTON, 0xd0d0d0);
	kos_WriteTextToWindow(92 + 160 + (60 - strlen(sLoad) * 6) / 2, panel_y + 16, 0x80, 0x000000, (char*)sLoad, 0);

	//new (clean)
	/*
	kos_DefineButton(90 + 160 + 70, panel_y + 9, 60, 20, NEW_BUTTON, 0xd0d0d0);
	kos_WriteTextToWindow(92 + 160 + 10 + 70, panel_y + 16, 0, 0x000000, (char*)sNew, strlen(sNew));
	*/

	if ((void*)edit_box_draw != NULL)
	{
		if (is_edit)
			edit_box_draw((DWORD)&cell_box);
		edit_box_draw((DWORD)&file_box);
	}

	draw_grid();
	sel_moved = 0;
}

void process_mouse()
{
	Dword mouse_btn, ckeys, shift, ctrl;
	int mouse_x, mouse_y, i, dx = 0, dy = 0;
	bool window_is_dragged=false;
	
	edit_box_mouse((dword)&cell_box);
	edit_box_mouse((dword)&file_box);

	int vert, hor;
	kos_GetScrollInfo(vert, hor);

	//sprintf(debuf, "scroll %U %U", vert, hor);
	//rtlDebugOutString(debuf);
		
	if (vert != 0)
	{
		stop_edit();
		scroll_y += vert;
		if (scroll_y<1) scroll_y=1;
		if (scroll_y>row_count-25) scroll_y=row_count-25;
		draw_grid();
		return;
	}

	if (!sel_moved && !size_state) //do not handle scrollbars when user selects cells
	{
		scrollbar_v_mouse((DWORD)&scroll_v);
		if (scroll_v.position != scroll_y-1)
		{
			scroll_y = scroll_v.position + 1;
			draw_grid();
		}

		scrollbar_h_mouse((DWORD)&scroll_h);
		if (scroll_h.position != scroll_x-1)
		{
			scroll_x = scroll_h.position + 1;
			draw_grid();
		}		
	}

	kos_GetMouseState(mouse_btn, mouse_x, mouse_y);
	mouse_x -= 5;
	mouse_y -= kos_GetSkinHeight();

	mouse_btn &= 0x0001;

	if (mouse_btn)
	{
		if (mouse_y < 0) return; // do nothing if mouse over header
		if (mouse_y > grid.y + grid.h) return;
	}

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
		else		// click on cell
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
					move_selection(kx, ky);
					//return;
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
		sel_moved = 0;		// for a good redraw
		//rtlDebugOutString("resize end");

		if (size_state == SIZE_DRAG)
		{
			fill_cells(sel_x, sel_y, sel_end_x, sel_end_y, old_end_x, old_end_y);
		}

		//sel_moved = (size_state == SIZE_SELECT && sel_x == sel_end_x && sel_y == sel_end_y && was_single_selection);
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
		else if (col_width[size_id] > grid.w / 2)
			col_width[size_id] = grid.w / 2;
		draw_size_grid();
	}
	if (size_state == SIZE_Y && mouse_y != size_mouse_y)
	{
		draw_size_grid();
		row_height[size_id] += mouse_y - size_mouse_y;
		if (row_height[size_id] < 15)
			row_height[size_id] = 15;
		else if (row_height[size_id] > grid.h / 2)
			row_height[size_id] = grid.h / 2;
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
				//sprintf(debuf, "yyy %U",col_left[i+1]);
				//rtlDebugOutString(debuf);
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
	int mouse_x, mouse_y, dx = 0, dy = 0;

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
		case 178:
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
		case 183:
		/*
			if (sel_y < row_count-(ny - scroll_y))	// page down
				dy = ny - scroll_y;
			else
				dy = row_count-(ny - scroll_y) - sel_y;
			dx = 0;
			redraw = 1;
		*/
			break;
		case 184:
		/*
			if (sel_y > ny - scroll_y)		// page up
				dy= - (ny - scroll_y);
			else
				dy = - (ny - scroll_y) + sel_y;
			dx = 0;
			redraw = 1;
		*/
			break;
		case 180: //home
			dx = -sel_x + 1;
			dy = 0;
			draw_grid();
			break;
		case 181: //end
			dx = col_count - (nx - scroll_x) - 1 - sel_x;
			dy = 0;
			draw_grid();
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
		case 22:	// contol-v
			{
				if (ctrl)
				{
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
					draw_grid();
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
					draw_grid();
					break;
				}
			}
		case 06:		// control-f
			{
				display_formulas = !display_formulas;
				draw_grid();
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
			if ((sel_end_x + dx) >= (col_count-1)) {dx=0;} //stub
			else if ((sel_end_y + dy) >= (row_count-1)) {dy=0;}
			else {
			move_selection(sel_x + dx, sel_y + dy);
			}
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
	if (!kos_GetButtonID(button)) return;

	// sprintf(debuf, "button %U", button);
	// rtlDebugOutString(debuf);

	switch (button)
	{
	case 1:
		kos_ExitApp();

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
		if (SaveFile(fname)) {
			kos_DrawBar(320, panel_y + 16, cWidth - 320, 12, PANEL_BG_COLOR);
			kos_WriteTextToWindow(320, panel_y + 16, 0x80, 0x000000, (char*)msg_save, 0);			
		}
		break;

	case LOAD_BUTTON:
		stop_edit();
		int r = LoadFile(fname);
		kos_DrawBar(320, panel_y + 16, cWidth - 320, 12, PANEL_BG_COLOR);
		char *result;
		if (r > 0)
		{
			calculate_values();
			sel_moved = 0;
			draw_window();
			result = (char*)msg_load;
		}
		else if (r == -1) result = (char*)er_file_not_found;
		else if (r == -2) result = (char*)er_format;
		kos_WriteTextToWindow(320, panel_y + 16, 0x80, 0x000000, result, 0);
		break;
	}
	if (button >= COL_HEAD_BUTTON && button < ROW_HEAD_BUTTON)
	{
		sel_end_x = sel_x = button - COL_HEAD_BUTTON;
		sel_y = 1;
		sel_end_y = row_count - 1;
		stop_edit();
		draw_grid();
		return;
	}
	else if (button >= ROW_HEAD_BUTTON && button < CELL_BUTTON)
	{
		sel_end_y = sel_y = button - ROW_HEAD_BUTTON;
		sel_x = 1;
		sel_end_x = col_count - 1;
		stop_edit();
		draw_grid();
		return;
	}

}

void kos_Main()
{
	kos_InitHeap();
	load_edit_box();
	init();
	kos_SetMaskForEvents(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	
	for (;;)
	{
		switch (kos_WaitForEvent())
		{
		case EM_MOUSE_EVENT:
			process_mouse();
			break;

		case EM_KEY_PRESS:
			process_key();
			break;

		case EM_BUTTON_CLICK:
			process_button();
			break;
		
		case EM_WINDOW_REDRAW:
			if (draw_and_define_window()) draw_window();
			break;
		}
	}
}


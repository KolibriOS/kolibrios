//если выделить область ячеек и сдвинуть курсор ввода с помощью клавиш, "следы" остануться
//нельзя перемещаться по буквам в редактируемой строке

#include "func.h"
#include "parser.h"
#include "calc.h"
#include "use_library.h"

#ifdef AUTOBUILD
extern char params[1024];
#endif
char params[1024];

#define TABLE_VERSION "0.99.3"

// strings
const char *sFileSign = "KolibriTable File\n";
const char sFilename[] = "Filename:";
const char sSave[] = "Save";
const char sLoad[] = "Load";
const char sNew[] = "New";

const char er_file_not_found[] = "'Cannot open file' -E";
const char er_format[] = "'Error: bad format' -E";
const char msg_save[] = "'File saved' -O";
//const char msg_load[] = "'File loaded' -O";
const char msg_save_error[] = "'Error saving file' -E";
const char msg_new[] = "'Memory cleared' -I";

// initial window sizes
#define WND_W 718
#define WND_H 514
// new window size and coordinates
int cWidth;
int cHeight;
kosSysColors sc;
// bottom panel
#define MENU_PANEL_HEIGHT 40

// interface colors
#define GRID_COLOR 0xa0a0a0
#define TEXT_COLOR 0x000000
#define CELL_COLOR 0xffffff
#define CELL_COLOR_ACTIVE 0xe0e0ff
#define HEADER_CELL_COLOR 0xE9E7E3
#define HEADER_CELL_COLOR_ACTIVE 0xC4C5BA //0xBBBBFF

// button IDs
#define SAVE_BUTTON 100
#define LOAD_BUTTON 101
#define NEW_BUTTON  102
#define SELECT_ALL_BUTTON 103

#define COL_BUTTON 0x100
#define ROW_BUTTON (COL_BUTTON + 0x100)
#define COL_HEAD_BUTTON (ROW_BUTTON + 0x100)
#define ROW_HEAD_BUTTON (COL_HEAD_BUTTON + 0x100)
#define CELL_BUTTON (ROW_HEAD_BUTTON + 0x100)

// editbox data
char edit_text[256];
edit_box cell_box = {0,9*8-6,WND_H - 16-32,0xffffff,0x94AECE,0,
	0x808080,0x10000000,sizeof(edit_text)-1,(dword)&edit_text,0,0};

// scrolls
#define SCROLL_SIZE 16
scroll_bar scroll_v = { SCROLL_SIZE,200,398, NULL, SCROLL_SIZE,0,115,
	15,0,0xeeeeee,0xD2CED0,0x555555,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};
scroll_bar scroll_h = { 200,NULL,SCROLL_SIZE, NULL, SCROLL_SIZE,0,115,
	15,0,0xeeeeee,0xD2CED0,0x555555,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

// ячейки - их параметры и текст
DWORD col_count = 100, row_count = 100;
DWORD *cell_w, *cell_h;
char ***cells;

struct GRID
{
	int x,y,w,h;
	int firstx, firsty; // cell x:y in the top left corner
} grid = {
	0,0,NULL,NULL,
	1,1
};

char ***values;	// значения формул, если есть

bool display_formulas = false;	// отображать ли формулы вместо значений

// координаты отображаемых столбцов и строк
DWORD *cell_x, *cell_y;

// буфер обмена
char ***buffer = NULL;
DWORD buf_col, buf_row;
DWORD buf_old_x, buf_old_y;

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
#define is_edit (cell_box.flags & ed_focus)

// редактирование имени файла
bool fn_edit = 0;
char fname[256];
edit_box file_box = {160,9*8+12,WND_H - 16-32,0xffffff,0x94AECE,
	0,0x808080,0x10000000,sizeof(fname)-1,(dword)&fname,0,0};

// изменение размеров
#define SIZE_X 1 // состояние
#define SIZE_Y 2
#define SIZE_SELECT 3
#define SIZE_DRAG 4
int size_mouse_x, size_mouse_y, size_id, size_state = 0;

// растаскивание ячейки при ее тащении за правый нижний угол, с заполнением ячеек
int drag_x, drag_y;
int old_end_x, old_end_y;

void draw_grid();
void EventGridSelectAll();

void DrawSelectedFrame(int x, int y, int w, int h, DWORD col)
{
	kos_DrawBar(x,y,w,2,col);          // up
	kos_DrawBar(x,y,2,h,col);          // left
	kos_DrawBar(x,y+h-2,w-2-3,2,col);  // bottom
	kos_DrawBar(x+w-2,y, 2,h-2-3,col); // right
	kos_DrawBar(x+w-4,y+h-4,4,4,col);
}

void DrawScrolls()
{
	// HOR
	scroll_h.x = 0;
	scroll_h.y = grid.y + grid.h;
	scroll_h.w = grid.w + SCROLL_SIZE + 1;
	scroll_h.all_redraw = true;
	scroll_h.max_area = col_count - 2;
	scroll_h.cur_area = nx-grid.firstx-1;
	scroll_h.position = grid.firstx-1;
	scrollbar_h_draw((DWORD)&scroll_h);

	// VER
	scroll_v.x = grid.x + grid.w;
	scroll_v.y = 0;
	scroll_v.h = grid.h + 1;
	scroll_v.all_redraw = true;
	scroll_v.max_area = row_count - 2;
	scroll_v.cur_area = ny-grid.firsty-1;
	scroll_v.position = grid.firsty-1;
	scrollbar_v_draw((DWORD)&scroll_v);
}


void start_edit(int x, int y)
{
	int ch = 0;
	if (x < grid.firstx || x > nx - 1)
	{
		grid.firstx = x;
		ch = 1;
	}
	if (y < grid.firsty || y > ny - 1)
	{
		grid.firsty = y;
		ch = 1;
	}
	if (ch)
	{
		sel_moved = 1;
		draw_grid();
	}

	file_box.flags &= ~ed_focus;

	cell_box.flags = ed_focus;
	cell_box.left = cell_x[x] + 1;
	cell_box.top = cell_y[y];
	cell_box.width = cell_w[x] - 2;
	memset((Byte*)edit_text, 0, sizeof(edit_text));
	if (cells[x][y])
	{
		strcpy(edit_text, cells[x][y]);
	}
	cell_box.offset = cell_box.shift = cell_box.shift_old = 0;
	cell_box.pos = cell_box.size = strlen(edit_text);

	edit_box_draw((DWORD)&cell_box);
	edit_box_draw((DWORD)&file_box);
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
	draw_grid();
}

void check_sel()
{
	DWORD sx0=grid.firstx, sy0=grid.firsty;

	if (sel_x >= nx - 1  /*&& sel_x < col_count - nx + grid.firstx + 1*/)
		//if (sel_x == nx)
			grid.firstx++;
		//else
		//	grid.firstx = sel_x;
	if (sel_y >= ny - 1 /*&& sel_y < row_count - ny + grid.firsty */)
		//if (sel_y == ny)
			grid.firsty++;
		//else
		//	grid.firsty = sel_y;

	if (sel_x < grid.firstx)
		grid.firstx = sel_x;
	if (sel_y < grid.firsty)
		grid.firsty = sel_y;

	if (sx0 != grid.firstx || sy0 != grid.firsty)
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
	draw_grid();
}

// x - между low и high ? - необязательно low<high
bool is_between(Dword x, Dword low, Dword high)
{
	return ((low<high)?(x >= low && x <= high):(x >= high && x <= low));
}

void clear_cell_slow(int px, int py)
{
	int i;
	int x0 = cell_w[0];
	for (i = grid.firstx; i < px; i++)
	{
		x0 += cell_w[i];
	}
	int x1 = x0;
	x1 += cell_w[px];
	int y0 = cell_h[0];
	for (i = grid.firsty; i < py; i++)
	{
		y0 += cell_h[i];
	}
	int y1 = y0;
	y1 += cell_h[py];
	kos_DrawBar(x0 + 1, y0 + 1, x1 - x0 - 1, y1 - y0 - 1, 0xffffff);
}



// рисование ячеек
#define is_x_changed(v) ((v) == sel_x || (v) == prev_x)
#define is_y_changed(v) ((v) == sel_y || (v) == prev_y)

void DrawCell(int x, int y, Dword w, Dword h, Dword id, Dword bg_color, char* text, bool header)
{
	bool small = false;
	if (x>grid.x+grid.w || w>grid.w || w<=0) return;
	if (x+w > grid.x + grid.w) {
		w = grid.x + grid.w - x;
		small = true;
	}
	if (y+h > grid.y + grid.h) {
		h = grid.y + grid.h - y;
		small = true;
	}
	kos_DrawBar(x, y, w, h, bg_color);
	if (!small) {
		if (id) kos_DefineButton(x+5, y, w-10, h-1, id+BT_NODRAW,0);
		if (header) kos_WriteTextToWindow( x + w/2 -strlen(text)*4, h/2-7+y, 0x90,TEXT_COLOR,text,0); //WriteTextCenter
		else kos_DrawCutTextSmall(x+3, h/2-7+y, w-7, TEXT_COLOR, text);
	}
}

void draw_grid()
{
	int i,j;
	long x0 = 0, y0 = 0, x = 0, y = 0;
	DWORD bg_color;
	kos_DrawBar(0,0,cell_w[0],cell_h[0],HEADER_CELL_COLOR); // left top cell
	kos_DefineButton(0,0,cell_w[0]-4,cell_h[0]-4, SELECT_ALL_BUTTON + BT_NODRAW, 0);

	//kos_DebugValue("sel_moved", sel_moved);

	nx=ny=0;

	// очистить область около выделенной ячейки
	if (sel_moved)
	{
		clear_cell_slow(sel_x, sel_y);
		clear_cell_slow(prev_x, prev_y);
	}
	else
	{
		// clean all cells
		//kos_DrawBar(cell_w[0]+1, cell_h[0]+1, grid.w - SCROLL_SIZE-cell_w[0]-1, he - SCROLL_SIZE-cell_h[0]-1, 0xffffff);
	}

	// column headers + vertical lines
	cell_x[0] = 0;
	x = cell_w[0]; 
	nx = 1;
	for (i = 1; i < col_count && x-x0 < grid.w; i++)
	{
		cell_x[i] = -1;
		if (i >= grid.firstx)
		{
			{				
				//if (!sel_moved || (is_x_changed(i))) {
					if (is_between(i,sel_x,sel_end_x)) bg_color = HEADER_CELL_COLOR_ACTIVE; else bg_color = HEADER_CELL_COLOR;
					kos_DrawBar(x-x0, 0, 1, grid.h, GRID_COLOR);
					DrawCell(x-x0+1, 0, cell_w[i]-1, cell_h[0], i+COL_HEAD_BUTTON, bg_color, cells[i][0], true);
				//}
				cell_x[i] = x - x0;
			}
		}
		else
		{
			x0 += cell_w[i];
		}
		x += cell_w[i];
		nx++;
	}

	// row headers + horizontal lines
	y = cell_h[0];
	ny = 1;
	cell_y[0] = 0;
	for (i = 1; i < row_count && y-y0 < grid.h; i++)
	{
		cell_y[i] = -1;
		if (i >= grid.firsty)
		{
			{
				//if (!sel_moved || (is_y_changed(i))) {
					if (is_between(i,sel_y,sel_end_y)) bg_color = HEADER_CELL_COLOR_ACTIVE; else bg_color = HEADER_CELL_COLOR;
					kos_DrawBar(0, y-y0, grid.w, 1, GRID_COLOR);
					DrawCell(0, y-y0+1, cell_w[0], cell_h[i]-1, i+ROW_HEAD_BUTTON, bg_color, cells[0][i], true);
				//}
				cell_y[i] = y - y0;
			}
		}
		else
		{
			y0 += cell_h[i];
		}
		y += cell_h[i];
		ny++;
	}
	
	// cells itself
	y = cell_h[0];
	for (i = grid.firsty; i < ny; i++)
	{
		x = cell_w[0];
		for (j = grid.firstx; j < nx; j++)
		{
			if (i && j)	//no need to draw headers one more
			{
				bool draw_frame_selection = false;
				bool error = false;
				bg_color = CELL_COLOR;

				if (is_between(j,sel_x,sel_end_x) && is_between(i, sel_y, sel_end_y)	// (j,i) - selected
				&& ((!sel_moved) || (is_x_changed(j) && is_y_changed(i))))			// and we must draw it
				{
					if (i == sel_y && j == sel_x)
					{
						draw_frame_selection = true;
						drag_x = x + cell_w[j] - 4;
						drag_y = y + cell_h[i] - 4;
					}
					else {
						bg_color = CELL_COLOR_ACTIVE; // selected but not main
					}
				}

				char *text;
				if (values[j][i] && values[j][i][0] == '#')
				{
					text = cells[j][i];
					error = true;
				}
				else {
					text = (values[j][i] && !display_formulas ? values[j][i] : cells[j][i]);
				}

				DrawCell(x+1, y+1, cell_w[j]-1, cell_h[i]-1, 0, bg_color, text, false);
				if (draw_frame_selection && j<nx-1 && i<ny-1) {
					DrawSelectedFrame(x+1,y, cell_w[j]-1, cell_h[i]+1, TEXT_COLOR);
				}
				else if (error) kos_DrawRegion(x+1, y+1, cell_w[j]-1, cell_h[i]-1, 0xff0000, 0);
			}
			x += cell_w[j];
		} 
		y += cell_h[i];
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

		x = cell_w[0]; 
		x0 = 0;
		for (i = 1; i < col_count && x - x0 + cell_w[i] < grid.w - 10; i++)
		{
			if (i >= grid.firstx)
			{
				if (i >= size_id)
					kos_DrawLine(x - x0, 0, x - x0, grid.h, 0, 1);
			}
			else
				x0 += cell_w[i];
			x += cell_w[i];
		}
		kos_DrawLine(x - x0, 0, x - x0, grid.h, 0, 1);
	}
	else
	{
		int y, y0, i;

		y = cell_h[0]; 
		y0 = 0;
		for (i = 1; i < col_count && y - y0 + cell_h[i] < grid.h - 10; i++)
		{
			if (i >= grid.firsty)
			{
				if (i >= size_id)
					kos_DrawLine(0, y - y0, grid.w, y - y0, 0, 1);
			}
			else
				y0 += cell_h[i];
			y += cell_h[i];
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

	DWORD x0 = cell_x[k0] - 1;
	DWORD x1 = cell_x[k1] + cell_w[k1] + 1;
	DWORD y0 = cell_y[n0] - 1;	
	DWORD y1 = cell_y[n1] + cell_h[n1] + 1;
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

void draw_window()
{
	kos_WindowRedrawStatus(1);
	kos_DefineAndDrawWindow(110,40,WND_W,WND_H,0x73,0x40FFFFFF,0,0,(Dword)"Table v" TABLE_VERSION);
	kos_WindowRedrawStatus(2); 

	kos_GetSystemColors(&sc);

	sProcessInfo info;
	kos_ProcessInfo(&info, 0xFFFFFFFF);
	cWidth = info.processInfo.width - 9;
	cHeight = info.processInfo.height - kos_GetSkinHeight() - 4;

	grid.x = 0;
	grid.y = 0;
	grid.w = cWidth - SCROLL_SIZE - 1;
	grid.h = cHeight - MENU_PANEL_HEIGHT - SCROLL_SIZE;

	if (info.processInfo.status_window&0x04) return; //draw nothing if window is rolled-up

	if (cWidth  < 430) { kos_ChangeWindow( -1, -1, 450, -1 ); return; }
	if (cHeight < 250) { kos_ChangeWindow( -1, -1, -1, 300 ); return; }

	sel_moved = 0;
	if (is_edit) stop_edit();

	int panel_y = cHeight - MENU_PANEL_HEIGHT + 1;
	kos_DrawBar(0, panel_y, cWidth, MENU_PANEL_HEIGHT-1, sc.work);
	kos_WriteTextToWindow(3 + 1, panel_y + 14, 0x90, sc.work_text, (char*)sFilename, 0);	
	
	file_box.top = panel_y + 10;
	file_box.width = cWidth - 265;
	int BTX = cWidth - 190;
	#define BTW 70
	//save
	kos_DefineButton(BTX + 25, file_box.top, BTW, 21, SAVE_BUTTON, sc.work);
	kos_WriteTextToWindow(BTX + 25 + (BTW - strlen(sSave) * 8) / 2, panel_y + 14, 0x90, sc.work_text, (char*)sSave, 0);
	//load
	kos_DefineButton(BTX + 25+BTW+5, file_box.top, BTW, 21, LOAD_BUTTON, sc.work);
	kos_WriteTextToWindow(BTX + 25+BTW+5 + (BTW - strlen(sLoad) * 8) / 2, panel_y + 14, 0x90, sc.work_text, (char*)sLoad, 0);
	// // new (clean)
	// kos_DefineButton(90 + 160 + 70, panel_y + 9, 60, 20, NEW_BUTTON, sc.work);
	// kos_WriteTextToWindow(92 + 160 + 10 + 70, panel_y + 16, 0, sc.work_text, (char*)sNew, strlen(sNew));

	if (sel_end_move) sel_moved = 0;
	draw_grid();
	sel_moved = 0;

	if (is_edit) edit_box_draw((DWORD)&cell_box);
	edit_box_draw((DWORD)&file_box);
}

void process_mouse()
{
	Dword mouse_btn, ckeys, shift, ctrl;

	int vert, hor;
	kos_GetScrollInfo(vert, hor);	
	if (vert != 0)
	{
		stop_edit();
		grid.firsty += vert;
		if (grid.firsty<1) grid.firsty=1;
		if (grid.firsty>row_count-25) grid.firsty=row_count-25;
		draw_grid();
		return;
	}

	if (!size_state) //do not handle scrollbars when user selects cells
	{
		if (!scroll_h.delta2) scrollbar_v_mouse((DWORD)&scroll_v);
		if (scroll_v.position != grid.firsty-1)
		{
			stop_edit();
			grid.firsty = scroll_v.position + 1;
			draw_grid();
		}

		if (!scroll_v.delta2) scrollbar_h_mouse((DWORD)&scroll_h);
		if (scroll_h.position != grid.firstx-1)
		{
			stop_edit();
			grid.firstx = scroll_h.position + 1;
			draw_grid();
		}
	}
	if (scroll_v.delta2 || scroll_h.delta2) return;

	if (is_edit) edit_box_mouse((dword)&cell_box);
	edit_box_mouse((dword)&file_box);

	int mouse_x, mouse_y, i;
	kos_GetMouseState(mouse_btn, mouse_x, mouse_y);
	mouse_x -= 5;
	mouse_y -= kos_GetSkinHeight();

	if (is_edit && mouse_x>=cell_box.left && mouse_x<=cell_box.left+cell_box.width 
		&& mouse_y>=cell_box.top && mouse_y<=cell_box.top+22) return;

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
		else if (mouse_y <= cell_h[0])
		{
			//rtlDebugOutString("can resize col_count");
			int kx = -1, i;
			for (i = 0; i < col_count - 1; i++)
			if (mouse_x >= cell_x[i] + cell_w[i] - 5 &&
				mouse_x <= cell_x[i + 1] + 5)
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
		else if (mouse_x <= cell_w[0])
		{
			int ky = -1;
			for (i = 0; i < row_count - 1; i++)
			if (mouse_y >= cell_y[i] + cell_h[i] - 5 &&
				mouse_y <= cell_y[i + 1] + 5)
			{
				ky = i; break;
			}
			if (ky != -1)
			{
				size_id = ky;
				size_state = SIZE_Y;
			}
		}
		else   // click on cell
		if (mouse_x <= cell_x[nx - 1] &&  mouse_y <= cell_y[ny - 1])
		{
			was_single_selection = sel_x == sel_end_x && sel_y == sel_end_y;
			int kx = -1, i;
			for (i = 0; i < col_count - 1; i++)
			if (mouse_x >= cell_x[i] &&
				mouse_x <= cell_x[i] + cell_w[i])
			{
				kx = i; break;
			}
			int ky = -1;
			for (i = 0; i < row_count - 1; i++)
			if (mouse_y >= cell_y[i] &&
				mouse_y <= cell_y[i] + cell_h[i])
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
		draw_grid();		// все сдвинулось - надо обновиться
		return;
	}
	if (size_state == SIZE_X && mouse_x != size_mouse_x)
	{
		draw_size_grid();
		cell_w[size_id] += mouse_x - size_mouse_x;
		if (cell_w[size_id] < 15)
			cell_w[size_id] = 15;
		else if (cell_w[size_id] > grid.w / 2)
			cell_w[size_id] = grid.w / 2;
		draw_size_grid();
	}
	if (size_state == SIZE_Y && mouse_y != size_mouse_y)
	{
		draw_size_grid();
		cell_h[size_id] += mouse_y - size_mouse_y;
		if (cell_h[size_id] < 15)
			cell_h[size_id] = 15;
		else if (cell_h[size_id] > grid.h / 2)
			cell_h[size_id] = grid.h / 2;
		draw_size_grid();
	}
	if ((size_state == SIZE_SELECT || size_state == SIZE_DRAG) && (mouse_x != size_mouse_x || mouse_y != size_mouse_y))
	{
		draw_drag();
		int kx = -1, i;
		for (i = 0; i < col_count - 1; i++)
			if (mouse_x >= cell_x[i] &&
				mouse_x <= cell_x[i + 1])
			{
				//sprintf(debuf, "yyy %U",cell_x[i+1]);
				//rtlDebugOutString(debuf);
				kx = i; break;
			}
		int ky = -1;
		for (i = 0; i < row_count - 1; i++)
			if (mouse_y >= cell_y[i] &&
				mouse_y <= cell_y[i + 1])
			{
				ky = i; break;
			}
		if (kx != -1) sel_end_x = kx;
		if (ky != -1) sel_end_y = ky;
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


void shift_selection(int dx, int dy, Dword shift) 
{
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


void process_key()
{
	Dword ckeys, shift, ctrl;
	dword key_editbox;
	Byte key_ascii, key_scancode;

	// key pressed, read it 
	ckeys = kos_GetSpecialKeyState();
	shift = ckeys & 0x3;
	ctrl = ckeys & 0x0c;
	sel_moved = 0;
	sel_end_move = 0;
	
	kos_GetKeys(key_editbox, key_ascii, key_scancode);

	if (cell_box.flags & ed_focus) {
		if (SCAN_CODE_ENTER == key_scancode) {
			stop_edit();
			draw_grid();
		}
		else if (SCAN_CODE_ESC == key_scancode) {
			cancel_edit();
		}
		else {
			__asm
			{
				mov eax, key_editbox
			}
			edit_box_key((dword)&cell_box);			
		}
	}
	else if (file_box.flags & ed_focus) {
		__asm
		{
			mov eax, key_editbox
		}
		edit_box_key((dword)&file_box);
		return;
	}
	else if (ctrl) {
		switch (key_scancode)
		{
			case SCAN_CODE_KEY_A:
				EventGridSelectAll();
				break;
			case SCAN_CODE_KEY_V:
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
				case SCAN_CODE_KEY_X:
				case SCAN_CODE_KEY_C:
				{
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
								if (SCAN_CODE_KEY_C == key_scancode)
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
					if (key_ascii == 24)     ///////WTF???? 
						calculate_values();
					draw_grid();
					break;
				}
			case SCAN_CODE_KEY_F:
				display_formulas = !display_formulas;
				draw_grid();
				break;
		}
	}
	else switch (key_scancode)
	{
		case SCAN_CODE_UP:
			shift_selection(0, -1, shift);
			break;
		case SCAN_CODE_LEFT:
			shift_selection(-1, 0, shift);
			break;
		case SCAN_CODE_RIGHT:
			shift_selection(1, 0, shift);
			break;
		case SCAN_CODE_DOWN:
			shift_selection(0, 1, shift);
			break;
		case SCAN_CODE_PGDN:
			shift_selection(0, ny-grid.firsty-1, shift);
			break;
		case SCAN_CODE_PGUP:
			shift_selection(0, -(ny-grid.firsty), shift);
			break;
		case SCAN_CODE_HOME:
			shift_selection(-sel_x + 1, 0, shift);
			break;
		case SCAN_CODE_END:
			shift_selection(col_count - (nx - grid.firstx) - 1 - sel_x, 0, shift);
			break;
		case SCAN_CODE_DEL:
			{
				int n0 = min(sel_x, sel_end_x);
				int n1 = max(sel_x, sel_end_x);
				int k0 = min(sel_y, sel_end_y);
				int k1 = max(sel_y, sel_end_y);

				for (int i = n0; i <= n1; i++)
					for (int j = k0; j <= k1; j++)
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
			break;
		case SCAN_CODE_F2:
			start_edit(sel_x, sel_y);
			break;
		case SCAN_CODE_F5:
			draw_grid();
			break;
		default:
			start_edit(sel_x, sel_y);
			__asm
			{
				mov eax, key_editbox
			}
			edit_box_key((dword)&cell_box);
			break;
	}
}

void EventLoadFile()
{
	stop_edit();
	int r = LoadFile(fname);
	if (r > 0) {
		calculate_values();
		sel_moved = 0;
		draw_grid();
		//result = (char*)msg_load;
	} else {
		char *result;
		if (r == -1) result = (char*)er_file_not_found;
		else if (r == -2) result = (char*)er_format;
		kos_AppRun("/sys/@notify", result);
	}
}

void EventGridSelectAll()
{
	sel_y = 1;
	sel_x = 1;
	sel_end_x = col_count - 1;
	sel_end_y = row_count - 1;
	stop_edit();
	draw_grid();
}

void process_button()
{
	Dword button;
	if (!kos_GetButtonID(button)) return;
	switch (button)
	{
	case 1:
		kos_ExitApp();

	case NEW_BUTTON:	// clear the table
		reinit();
		draw_grid();
		break;

	case SAVE_BUTTON:
		stop_edit();
		if (SaveFile(fname)) {
			kos_AppRun("/sys/@notify", (char*)msg_save);
		}
		else {
			kos_AppRun("/sys/@notify", (char*)msg_save_error);
		}
		break;

	case LOAD_BUTTON:
		EventLoadFile();
		break;

	case SELECT_ALL_BUTTON:
		EventGridSelectAll();
		break;
	}
	if (button >= COL_HEAD_BUTTON    &&    button < ROW_HEAD_BUTTON)
	{
		sel_end_x = sel_x = button - COL_HEAD_BUTTON;
		sel_y = 1;
		sel_end_y = row_count - 1;
		stop_edit();
		draw_grid();
		return;
	}
	else if (button >= ROW_HEAD_BUTTON    &&    button < CELL_BUTTON)
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
	if (params[0]) {
		strcpy(fname, params);
		file_box.size = file_box.pos = strlen(fname);
		EventLoadFile();
	}
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
			draw_window();
			break;
		}
	}
}


//#include "strings.h"

void DrawRectangle(dword x,y,w,h,color1)
{
	if (w<=0) || (h<=0) return;
	DrawBar(x,y,w,1,color1);
	DrawBar(x,y+h,w,1,color1);
	DrawBar(x,y,1,h,color1);
	DrawBar(x+w,y,1,h+1,color1);
}

void DrawRectangle3D(dword x,y,w,h,color1,color2)
{
	if (w<=0) || (h<=0) return;
	DrawBar(x,y,w+1,1,color1);
	DrawBar(x,y+1,1,h-1,color1);
	DrawBar(x+w,y+1,1,h,color2);
	DrawBar(x,y+h,w,1,color2);
}

void DrawCaptButton(dword x,y,w,h,id,color_b, color_t,text)
{
	DefineButton(x,y,w,h,id,color_b);
	WriteText(-strlen(text)*6+w/2+x+1,h/2-3+y,0x80,color_t,text);
}

void DrawCircle(int x, y, r)
{
	int i;
	float px=0, py=r, ii = r * 3.1415926 * 2;
	FOR (i = 0; i < ii; i++)
	{
        PutPixel(px + x, y - py, 0);
        px = py / r + px;
        py = -px / r + py;
	}
}

void CheckBox(dword x,y,w,h, bt_id, text, graph_color, text_color, is_checked)
{
	DefineButton(x-1, y-1, strlen(text)*6 + w + 17, h+2, bt_id+BT_HIDE+BT_NOFRAME, graph_color);
	WriteText(x+w+10, h / 2 + y -3, 0x80, text_color, text);
	DrawRectangle(x, y, w, h, graph_color);
	if (is_checked == 1)
	{
		DrawRectangle(x+1, y+1, w-2, h-2, 0xffffff);
		DrawBar(x+2, y+2, w-3, h-3, graph_color);	
		return; //не дадим стрелять себе в ногу
	}
	if (is_checked == 2) //not active
	{
		DrawRectangle(x+1, y+1, w-2, h-2, 0xffffff);
		DrawBar(x+2, y+2, w-3, h-3, 0x888888);	
		return;
	} 
	else
	{
		DrawRectangle3D(x+1, y+1, w-2, h-2, 0xDDDddd, 0xffffff);
		DrawBar(x+2, y+2, w-3, h-3, 0xffffff);
	} 
}

void DrawProgressBar(dword st_x, st_y, st_w, st_h, col_fon, col_border, col_fill, col_text, progress_percent, status_text)
{
	int progress_w;
	static int fill_old;
	    
	if (progress_percent<=0) {DrawBar(st_x,st_y, st_x + st_w + fill_old + 15,st_h+1, col_fon); fill_old=0; return;}
	
	DrawRectangle(st_x, st_y, st_w,st_h, col_border);
	DrawRectangle3D(st_x+1, st_y+1, st_w-2,st_h-2, 0xFFFfff, 0xFFFfff);

	if (progress_percent>0) && (progress_percent<=100)
	{
		progress_w = st_w - 3 * progress_percent / 100;
		DrawBar(st_x+2, st_y+2, progress_w, st_h-3, col_fill);
		DrawBar(st_x+2+progress_w, st_y+2, st_w-progress_w-3, st_h-3, 0xFFFfff);
	}
	
	if (status_text)
	{
		DrawBar(st_x+st_w+15, st_h/2-4+st_y, fill_old, 9, col_fon);
		WriteText(st_x+st_w+15, st_h/2-4+st_y, 0x80, col_text, status_text);
		fill_old = strlen(status_text) * 6;
	}
}
void DrawRectangle(dword x,y,width,height,color1)
{
	DrawBar(x,y,width,1,color1);
	DrawBar(x,y+height,width,1,color1);
	DrawBar(x,y,1,height,color1);
	DrawBar(x+width,y,1,height+1,color1);
}

void DrawRectangle3D(dword x,y,width,height,color1,color2)
{
	DrawBar(x,y,width+1,1,color1);
	DrawBar(x,y+1,1,height-1,color1);
	DrawBar(x+width,y+1,1,height,color2);
	DrawBar(x,y+height,width,1,color2);
}

void DrawCaptButton(dword x,y,width,height,id,color_b, color_t,text,text_len)
{
	DefineButton(x,y,width,height,id,color_b);
	WriteText(-text_len*6+width/2+x+1,height/2-3+y,0x80,color_t,text);
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
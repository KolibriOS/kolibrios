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
	WriteText(-text_len*6+width/2+x+1,height/2-3+y,0x80,color_t,text,0);
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

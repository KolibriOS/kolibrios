#define COL_PADDING 0xC8C9C9

dword col_palette[14] = {0xD2D3D3,0xD4D4D4,0xD6D5D6,0xD8D7D8,0xDAD8D9,0xDCDADB,
0xDFDCDD,0xE1DDDE,0xE2DEE0,0xE4DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1};

void DrawFlatButton(dword x,y,width,height,id,color,text)
{
	int fill_h;
	DrawRectangle(x,y,width,height,sc.work_graph);
	DrawRectangle3D(x+1,y+1,width-2,height-2,0xFEFEFE,COL_PADDING);
	PutPixel(x+width-1, y+1, COL_PADDING);
	if (color!=-1) DrawFilledBar(x+2, y+2, width-3, height-3);
	IF (id<>0)	DefineButton(x+1,y+1,width-2,height-2,id+BT_HIDE,0xEFEBEF);
	WriteText(-strlen(text)*6+width/2+x+1,height/2-3+y,0x80,sc.work_text,text);
}

void DrawFilledBar(dword x, y, w, h)
{
	int i, fill_h;
	if (h <= 14) fill_h = h; else fill_h = 14;
	for (i=0; i<fill_h; i++) DrawBar(x, y+i, w, 1, col_palette[14-i]);	
	DrawBar(x, y+i, w, h-fill_h, col_palette[14-i]);		
}

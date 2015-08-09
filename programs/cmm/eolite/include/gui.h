
dword col_palette[14] = {0xD2D3D3,0xD4D4D4,0xD6D5D6,0xD8D7D8,0xDAD8D9,0xDCDADB,
0xDFDCDD,0xE1DDDE,0xE2DEE0,0xE4DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1};

void Scroll() {
	dword i;

	word sc_x = files.x + files.w;
	word sc_y = files.y;
	word sc_h = files.h - 16;
	word sc_slider_y;

	if (files.count<=0)
	{
		sc_slider_y = sc_y;
		sc_slider_h = sc_h - 1;
	}
	else
	{
		sc_slider_y = files.first * sc_h / files.count + sc_y;
		sc_slider_h = sc_h * files.visible - files.visible / files.count;
		if (sc_slider_h < 20) sc_slider_h = 20; //minimal scroll width
		if (sc_slider_h > sc_h-sc_slider_y+56) || (files.first+files.visible>=files.count) sc_slider_y=onTop(23+sc_slider_h,0); //для большого списка 
	}
	//slider
	DrawFlatButton(sc_x,sc_slider_y,16,sc_slider_h,0,-1,"");
	if (!scroll_used) for (i=0; i<13; i++) DrawBar(sc_x + 2 + i, sc_slider_y+2, 1, sc_slider_h-3, col_palette[13-i]);
	if (scroll_used)  for (i=0; i<13; i++) DrawBar(sc_x + 2 + i, sc_slider_y+2, 1, sc_slider_h-3, col_palette[i]);
	//area before slider
	if (sc_slider_y > sc_y + 1) 
	{
		DrawBar(sc_x+1, sc_y,   15, 1, 0xC7C9C9);
		DrawBar(sc_x+1, sc_y+1,  1, sc_slider_y-sc_y-1, 0xC7C9C9);
		DrawBar(sc_x+2, sc_y+1, 14, sc_slider_y-sc_y-1, 0xCED0D0);
	}
	//area after slider
	if (sc_h-sc_slider_h+sc_y-2>sc_slider_y)
	{
		DrawBar(sc_x+1, sc_slider_y + sc_slider_h+1, 15, 1, 0xC7C9C9);
		DrawBar(sc_x+1, sc_slider_y + sc_slider_h+2,  1, sc_h-sc_slider_h-sc_slider_y+sc_y-2, 0xC7C9C9);
		DrawBar(sc_x+2, sc_slider_y + sc_slider_h+2, 14, sc_h-sc_slider_h-sc_slider_y+sc_y-2, 0xCED0D0);
	}
}

void DrawFlatButton(dword x,y,width,height,id,color,text)
{
	int fill_h;
	DrawRectangle(x,y,width,height,system.color.work_graph);
	DrawRectangle3D(x+1,y+1,width-2,height-2,0xFEFEFE,col_padding);
	PutPixel(x+width-1, y+1, col_padding);
	if (color!=-1) DrawFilledBar(x+2, y+2, width-3, height-3);
	IF (id<>0)	DefineButton(x+1,y+1,width-2,height-2,id+BT_HIDE,0xEFEBEF);
	WriteText(-strlen(text)*6+width/2+x+1,height/2-3+y,0x80,system.color.work_text,text);
}

void DrawFilledBar(dword x, y, w, h)
{
	int i, fill_h;
	if (h <= 14) fill_h = h; else fill_h = 14;
	for (i=0; i<fill_h; i++) DrawBar(x, y+i, w, 1, col_palette[14-i]);	
	DrawBar(x, y+i, w, h-fill_h, col_palette[14-i]);		
}

void ShowMessage(dword message, pause_duration)
{
	int form_x=files.w-220/2+files.x;
	int form_y=160;
	DrawPopup(form_x,form_y,220,80,1,system.color.work,system.color.work_graph);
	WriteText(-strlen(message)*3+110+form_x,80/2-4+form_y,0x80,system.color.work_text,message);
	pause(pause_duration);
	if (pause_duration) List_ReDraw();
}
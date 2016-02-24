
dword col_palette[14] = {0xD2D3D3,0xD4D4D4,0xD6D5D6,0xD8D7D8,0xDAD8D9,0xDCDADB,0xDFDCDD,0xE1DDDE,0xE2DEE0,0xE4DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1};

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
		if (sc_slider_h > sc_h-sc_slider_y+56) || (files.first+files.visible>=files.count) sc_slider_y= Form.cheight - 19 - sc_slider_h; //для большого списка 
	}
	//slider
	DrawRectangle(sc_x,sc_slider_y,16,sc_slider_h,col_graph);
	DrawRectangle3D(sc_x+1,sc_slider_y+1,14,sc_slider_h-2,0xFEFEFE,col_padding);
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

void DrawFlatButton(dword x,y,width,height,id,text)
{
	DrawRectangle(x,y,width,height,col_graph);
	DrawRectangle3D(x+1,y+1,width-2,height-2,0xFEFEFE,col_padding);
	PutPixel(x+width-1, y+1, col_padding);
	DrawFilledBar(x+2, y+2, width-3, height-3);
	if (id) DefineButton(x+1,y+1,width-2,height-2,id+BT_HIDE,0xEFEBEF);
	if (height<18) {
		WriteText(-strlen(text)*6+width/2+x+1,height/2+y-3,0x80,MixColors(system.color.work_text,0xFFFfff,210),text);
	}
	else {
		DrawRectangle3D(x-1,y-1,width+2,height+2,system.color.work,MixColors(system.color.work,col_graph,200));
		WriteText(-strlen(text)*8+width/2+x+1,height/2+y-6,0x90,MixColors(system.color.work_text,0xFFFfff,210),text);
	}
}


void DrawFilledBar(dword x, y, w, h)
{
	int i, fill_h;
	if (h <= 14) fill_h = h; else fill_h = 14;
	for (i=0; i<fill_h; i++) DrawBar(x, y+i, w, 1, col_palette[14-i]);
	DrawBar(x, y+i, w, h-fill_h, col_palette[14-i]);
}

int popin_w=250;
void DrawEolitePopup(dword b1_text, b2_text)
{
	int button_padding=30;
	int b1_len = strlen(b1_text) * 8 + button_padding;
	int b2_len = strlen(b2_text) * 8 + button_padding;
	int popin_x = files.w - popin_w / 2 + files.x ;
	int button_margin = popin_w - b1_len - b2_len / 3;
	int b1_x = popin_x + button_margin;
	int b2_x = popin_x + button_margin + b1_len + button_margin;
	DrawPopup(popin_x, 160, popin_w, 90, 1, system.color.work, col_graph);
	DrawFlatButton(b1_x, 210, b1_len, 24, POPUP_BTN1, b1_text);
	DrawFlatButton(b2_x, 210, b2_len, 24, POPUP_BTN2, b2_text);
}
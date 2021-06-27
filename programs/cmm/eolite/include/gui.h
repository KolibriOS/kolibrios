
void DrawScroll(bool _scroll_used) {
	dword i;

	dword sc_x = files.x + files.w;
	dword sc_y = files.y;
	dword sc_h = files.h - 16;
	dword sc_slider_y;
	dword sc_center;

	if (files.count<=0)
	{
		sc_slider_y = sc_y - 1;
		sc_slider_h = sc_h + 1;
	} else {
		sc_slider_y = files.first * sc_h / files.count + sc_y - 1;
		sc_slider_h = sc_h * files.visible - files.visible / files.count + 2;
		if (sc_slider_h < 20) {
			sc_slider_h = 20; //set minimal scroll height
		}
		if (sc_slider_h > sc_h-sc_slider_y+56) || (files.first+files.visible>=files.count) {
			sc_slider_y= sc_y + sc_h - sc_slider_h; //fix for the very long list
		}
	}
	//slider
	DrawRectangle(sc_x,sc_slider_y,16,sc_slider_h,sc.work_graph);
	DrawRectangle3D(sc_x+1,sc_slider_y+1,14,sc_slider_h-2, sc.work_light , sc.work_dark);
	for (i=0; i<13; i++) {
		if (!_scroll_used) EDX = col.work_gradient[13-i]; else EDX = col.work_gradient[i];
		DrawBar(sc_x + 2 + i, sc_slider_y+2, 1, sc_slider_h-3, EDX);
	}

	sc_center = sc_slider_h / 2 + sc_slider_y;

	DrawBar(sc_x+4,   sc_center-3, 8,  1, sc.work_graph);
	DrawBar(sc_x+4+1, sc_center-2, 8,  1, sc.work_light);

	DrawBar(sc_x+3,   sc_center,   10, 1, sc.work_graph);
	DrawBar(sc_x+3+1, sc_center+1, 10, 1, sc.work_light);

	DrawBar(sc_x+4,   sc_center+3, 8,  1, sc.work_graph);
	DrawBar(sc_x+4+1, sc_center+4, 8,  1, sc.work_light);

	//area before slider
	if (sc_slider_y > sc_y + 1) 
	{
		DrawBar(sc_x+1, sc_y,   15, 1, col.slider_bg_left);
		DrawBar(sc_x+1, sc_y+1,  1, sc_slider_y-sc_y-1, col.slider_bg_left);
		DrawBar(sc_x+2, sc_y+1, 14, sc_slider_y-sc_y-1, col.slider_bg_big);
	}
	//area after slider
	if (sc_h-sc_slider_h+sc_y-2>sc_slider_y)
	{
		DrawBar(sc_x+1, sc_slider_y + sc_slider_h+1, 15, 1, col.slider_bg_left);
		DrawBar(sc_x+1, sc_slider_y + sc_slider_h+2,  1, sc_h-sc_slider_h-sc_slider_y+sc_y-2, col.slider_bg_left);
		DrawBar(sc_x+2, sc_slider_y + sc_slider_h+2, 14, sc_h-sc_slider_h-sc_slider_y+sc_y-2, col.slider_bg_big);
	}
}

void DrawFlatButtonSmall(dword x,y,width,height,id,text)
{
	DrawRectangle(x,y,width,height,sc.work_graph);
	DrawRectangle3D(x+1,y+1,width-2,height-2, sc.work_light, sc.work_dark);
	PutPixel(x+width-1, y+1, sc.work_dark);
	DrawFilledBar(x+2, y+2, width-3, height-3);
	if (id) DefineHiddenButton(x+1,y+1,width-2,height-2,id);
	WriteText(-strlen(text)*6+width/2+x+1,height/2+y-3,0x80,sc.work_text,text);
}

void DrawFuncButton(dword x,y,width,id,number,text)
{
	#define FW 12
	#define FH 16
	int numw = calc(number/10)*6+FW;
	if (skin_is_dark()) {
		DrawFlatButtonSmall(x,y,width,FH,id,text);
		return;
	}
	DrawRectangle(x,y,width,FH,sc.work_graph);
	DrawRectangle3D(x+1,y+1,width-2,FH-2, 0x97D194, 0x00A100);
	PutPixel(x+width-1, y+1, sc.work_dark);
	DrawBar(x+2, y+2, numw, FH-2, 0x6060FF);
	WriteText(x+6,FH/2+y-2,0x80,0x444444,itoa(number));
	$sub ebx, 1 <<16 + 1
	$add ecx, 0xFFFfff-0x444444
	$int 64
	DrawBar(x+2+numw, y+2, width-3-numw, FH-3, 0x00AA00);
	DefineHiddenButton(x+1,y+1,width-2,FH-2,id);
	WriteText(-strlen(text)*6+width/2+x+8,FH/2+y-2,0x80,0x444444,text);
	$sub ebx, 1 <<16 + 1
	$add ecx, 0xFFFfff-0x444444
	$int 64
}

void DrawFilledBar(dword x, y, w, h)
{ int i; for (i=0; i<h; i++) DrawBar(x, y+h-i-1, w, 1, col.work_gradient[i]); }

void DrawEolitePopup(dword b1_text, b2_text)
{
	#define POPIN_W 260
	int popin_x = files.w - POPIN_W / 2 + files.x ;
	DrawPopup(popin_x, 160, POPIN_W, 95, 1, sc.work, sc.work_graph);
	DrawCaptButton(popin_x+23+000, 215, 100, 26, POPUP_BTN1, sc.button, sc.button_text, b1_text);
	DrawCaptButton(popin_x+23+114, 215, 100, 26, POPUP_BTN2, sc.button, sc.button_text, b2_text);
}

void DrawDot(dword x,y) {
	dword col_pxl = MixColors(sc.work_graph, sc.work, 60);
	DrawBar(x+1,y,2,4,sc.work_graph);
	DrawBar(x,y+1,4,2,sc.work_graph);
	PutPixel(x,y,col_pxl);
	PutPixel(x+3,y,col_pxl);
	PutPixel(x,y+3,col_pxl);
	PutPixel(x+3,y+3,col_pxl);
}

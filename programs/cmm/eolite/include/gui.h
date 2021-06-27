
//===================================================//
//                                                   //
//                     SCROLL                        //
//                                                   //
//===================================================//

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

	DrawBar(sc_x+3,   sc_center,   10, 1, sc.work_graph);
	DrawBar(sc_x+4,   sc_center-3, 8,  1, sc.work_graph);
	DrawBar(sc_x+4,   sc_center+3, 8,  1, sc.work_graph);

	DrawBar(sc_x+3+1, sc_center+1, 10, 1, sc.work_light);
	DrawBar(sc_x+4+1, sc_center-2, 8,  1, sc.work_light);
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

//===================================================//
//                                                   //
//                  FLAT BUTTONS                     //
//                                                   //
//===================================================//

void DrawFlatButtonSmall(dword x,y,width,height,id,text)
{
	DrawRectangle(x,y,width,height,sc.work_graph);
	DrawRectangle3D(x+1,y+1,width-2,height-2, sc.work_light, sc.work_dark);
	PutPixel(x+width-1, y+1, sc.work_dark);
	DrawFilledBar(x+2, y+2, width-3, height-3);
	if (id) DefineHiddenButton(x+1,y+1,width-2,height-2,id);
	WriteText(-strlen(text)*6+width/2+x+1,height/2+y-3,0x80,sc.work_text,text);
}

void DrawFilledBar(dword x, y, w, h)
{ int i; for (i=0; i<h; i++) DrawBar(x, y+h-i-1, w, 1, col.work_gradient[i]); }

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

//===================================================//
//                                                   //
//                     LEFT PANEL                    //
//                                                   //
//===================================================//

void Tip(int y, dword caption, id, arrow)
{
	DrawBar(17,y,160,1,col.list_bg);
	DrawBar(17,y+1,1,16,sc.work_light);
	DrawFilledBar(18, y+1, 160-1, 16);
	DrawBar(17,y+17,160,1,sc.work_graph);
	WriteText(25,y+5,0x80,col.list_gb_text,caption);
	if (id) DefineButton(159,y+1,16,16,id+BT_HIDE+BT_NOFRAME,0); //arrow button
	WriteText(165,y+5,0x80,col.list_gb_text,arrow); //arrow
}


void ActionsDraw(int actions_y)
{
	int i;
	Tip(actions_y-18, T_ACTIONS, 77, "");
	for (i=0; actions[i*3]!=0; i++, actions_y+=DEV_H)
	{
		DrawBar(17,actions_y,160,DEV_H,0xFFFFFF); //белое
		DefineButton(17,actions_y,159,DEV_H,actions[i*3]+BT_HIDE,0xE4DFE1);
		WriteText(45,actions_y+4,0x80,0,actions[i*3+1]);
		WriteText(-strlen(actions[i*3+2])*6+170,actions_y+4,0x80,0x999999,actions[i*3+2]);
		_PutImage(23,actions_y+2, 14,13, i*14*13*3+#factions);
	}
}

void DrawLeftPanelBg(int actions_y)
{
	int start_y = actions_y+159;
	int area_h;
	DrawBar(2,41,190,15,waves_pal[0]);    //above devices block
	DrawBar(17,actions_y+75,160,15,EDX);  //below devices block
	DrawBar(2,56,15,actions_y+103,EDX);   //on the left
	DrawBar(177,56,15,actions_y+103,EDX); //on the right
	area_h = Form.cheight-start_y-2 - status_bar_h;
	if (area_h < 268){
		PutPaletteImage(#blue_hl, 190, area_h, 2, start_y, 8, #waves_pal);
	} else {
		DrawBar(2,start_y,190, area_h-268, waves_pal[0]);
		PutPaletteImage(#blue_hl, 190, 268, 2, Form.cheight-270-status_bar_h, 8, #waves_pal);
	}
	PutShadow(17,actions_y+75,160,1,1,3);
	PutShadow(18,actions_y+75+1,158,1,1,1);
	PutShadow(17,start_y,160,1,1,3);
	PutShadow(18,start_y+1,158,1,1,1);
}

//===================================================//
//                                                   //
//                     PATHBAR                       //
//                                                   //
//===================================================//

char work_area_pointer[1024];
PathShow_data PathShow = {0, 17,250, 6, 250, 0, 0, 0x000000, 0xFFFFCC, NULL, #work_area_pointer, 0};
void DrawPathBar()
{
	if (efm) {
		DrawPathBarKfm();
	} else {
		DrawPathEolite();
	}
}

void DrawPathEolite()
{
	if (show_breadcrumb.checked) {
		DrawBreadCrumbs(); 
		return;
	}

	PathShow.text_pointer = location[0];
	PathShow.start_x = 250;
	PathShow.start_y = 17;
	PathShow.area_size_x = Form.cwidth-300;
	DrawBar(PathShow.start_x-3, PathShow.start_y-6, PathShow.area_size_x+3, 19, col.odd_line);
	DrawRectangle(PathShow.start_x-4,PathShow.start_y-7,PathShow.area_size_x+4,20,sc.work_graph);
	DefineHiddenButton(PathShow.start_x-4+1,PathShow.start_y-7+1,PathShow.area_size_x+4-2,20-2,PATH_BTN);
	DrawBar(PathShow.start_x-4, PathShow.start_y+14, PathShow.area_size_x+5+18, 1, sc.work_light);

	DrawFlatButtonSmall(PathShow.start_x+PathShow.area_size_x,PathShow.start_y-7,18,20, 61, "\x19");

	PathShow.font_color = col.list_gb_text;
	PathShow_prepare stdcall(#PathShow);
	PathShow_draw stdcall(#PathShow);
}

void DrawPathBarKfm()
{
	dword back_color, text_color;
	int draw_x, draw_w;
	int i=0;
	if (!Form.cwidth) return;

	if (skin_is_dark()) {
		back_color = col.odd_line;
		text_color = col.list_gb_text;
	} else {
		back_color = 0xFFFFCC; 
		text_color = 0x222222;
	}
	draw_x = 3 + DDW;
	draw_w = Form.cwidth/2 - draw_x - 17;
	do {
		DrawBar(draw_x, SELECTY-1, draw_w-KFM2_DEVH+1, 1, sc.work_graph);
		DrawBar(draw_x, SELECTY,   draw_w-KFM2_DEVH+1, KFM2_DEVH, back_color);
		DrawBar(draw_x, SELECTY+KFM2_DEVH, draw_w-KFM2_DEVH+1, 1, sc.work_graph);
		kfont.WriteIntoWindow(draw_x + 3, math.max(KFM2_DEVH-kfont.height/2+SELECTY,0), 
			back_color, text_color, kfont.size.pt, location[i]+strrchr(location[i], '/'));
		DrawFlatButtonSmall(draw_x+draw_w-KFM2_DEVH+1, SELECTY-1, KFM2_DEVH-1, KFM2_DEVH+1, 576+i, "\x19");
		draw_x = Form.cwidth/2 + DDW + 1;
		draw_w = Form.cwidth - draw_x - 2;
		i++;
	} while (i<2);
}

//===================================================//
//                                                   //
//                   BREADCRUMBS                     //
//                                                   //
//===================================================//

void DrawBreadCrumbs()
 collection_int breadCrumb=0;
 char PathShow_path[4096];
 block btn;
 int i;
 unsigned text_line;
 {
	strcat(#PathShow_path, path);
	for (i=0; i<50; i++) DeleteButton(i+BREADCRUMB_ID);
	breadCrumb.drop();
	for (i=0; (PathShow_path[i]) && (i<sizeof(PathShow_path)-1); i++) 
	{
		if (PathShow_path[i]=='/') {
			PathShow_path[i] = NULL;
			breadCrumb.add(i+1);
		}
	}
	breadCrumb.add(i+1);
	btn.set_size(246,10,NULL,20);
	//area_w = Form.cwidth - btn.x - 20;
	for (i=0; i<breadCrumb.count-1; i++)
	{
		text_line = breadCrumb.get(i) + #PathShow_path;
		btn.w = strlen(text_line)*8+10;
		DrawBreadcrumbButton(btn.x, btn.y, btn.w, btn.h, i+BREADCRUMB_ID, text_line);
		btn.x += btn.w;
	}
	btn.x++;
	DrawBar(btn.x,btn.y-1,Form.cwidth-btn.x-25,btn.h+3,sc.work);
}

void ClickOnBreadCrumb(unsigned clickid)
{
	int i, slashpos = path;
	for (i=0; i!=clickid+2; i++) {
		slashpos=strchr(slashpos,'/')+1;
	}
	ESBYTE[slashpos-1] = NULL;
	Open_Dir(path,WITH_REDRAW);
}

void DrawBreadcrumbButton(dword x,y,w,h,id,text)
{
	int i;
	DrawRectangle(x,y,w,h,sc.work_graph);
	for (i=0; (i<h-1) & (i<20); i++) DrawBar(x+1, y+i+1, w-1, 1, col.work_gradient[20-i]);
	DrawRectangle3D(x+1,y+1,w-2,h-2,sc.work_light, sc.work_dark);
	DefineHiddenButton(x+1,y+1,w-2,h-2,id);
	WriteText(-strlen(text)*8+w/2+x,h/2+y-7,0x90,sc.work_text,text);
	DrawBar(x, y+h+1, w+1, 1, sc.work_light);
}

//===================================================//
//                                                   //
//                       MISC                        //
//                                                   //
//===================================================//

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
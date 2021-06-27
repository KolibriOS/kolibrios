
struct _SystemDiscs
{
	collection list;
	int dev_num;
	void Get();
	void Draw();
	void DrawSelect();
	void DrawOptions();
	void Click();
} SystemDiscs=0;

void _SystemDiscs::Get()
{
	bool kolibrios_exists=false;
	char dev_name[10], sys_discs[10];
	int i1, j1, dev_num_i, dev_disc_num;
	dword devbuf, diskbuf;

	list.drop();
	devbuf = malloc(10000);
	ReadDir(19, devbuf, "/");
	dev_num = dev_num_i = EBX;
	for (i1=0; i1<dev_num_i; i1++)
	{
		sprintf(#dev_name,"/%s",i1*304+ devbuf+72);
		GetDir(#diskbuf, #dev_disc_num, #dev_name, DIRS_NOROOT);
		for (j1=0; j1<dev_disc_num; j1++;)
		{
			sprintf(#sys_discs,"%s/%s",#dev_name,j1*304+ diskbuf+72);
			if (sys_discs[1]=='c') || (dir_exists(#sys_discs)) list.add(#sys_discs);
		}
		if (!strcmp(#sys_discs, "/rd/1")) 
		{
			if (dir_exists("/kolibrios")) && (!kolibrios_exists) {
				kolibrios_exists=true;
				list.add("/kolibrios");
				dev_num++;
			}
		}
	}
	free(devbuf);
	free(diskbuf);
}

void GetDiskIconAndName(char disk_first_letter, dword dev_icon, dword disc_name)
{
	int i;
	for (i=0; devinfo[i]!=0; i+=3) {
		if (disk_first_letter == ESBYTE[devinfo[i]]) {
			ESBYTE[dev_icon] = devinfo[i+1];
			strcpy(disc_name, devinfo[i+2]);
			return;
		}
	}
	ESBYTE[dev_icon]=5;
	strcpy(disc_name, T_UNC);
}

#define DEV_H 17
#define DDW 120
#define KFM2_DEVH 20

void _SystemDiscs::Draw()
{    
	char dev_name[15], disc_name[100], i, dev_icon;
	bool is_active=0;
	int draw_y, draw_x;
	
	for (i=0; i<30; i++) DeleteButton(100+i);

	if (efm) { 
		DrawSelect(Form.cwidth/2-DDW, KFM_DEV_DROPDOWN_1, location[0]);
		DrawSelect(Form.cwidth-DDW-2, KFM_DEV_DROPDOWN_2, location[sizeof(dword)]);
		files.y = 40 + 17;
	} else { 
		draw_y = 74; 
		draw_x = 17; 
		for (i=0;i<list.count;i++) {
			strcpy(#dev_name, list.get(i));
			GetDiskIconAndName(dev_name[1], #dev_icon, #disc_name);
			if (strstr(path, #dev_name)==path) is_active=true; else is_active=false;

			DrawBar(draw_x,draw_y,6,DEV_H+1,0xFFFFFF);
			DrawBar(draw_x+6+18,draw_y,160-6-18,DEV_H+1,0xFFFFFF);
			DefineHiddenButton(draw_x,draw_y,159,16,100+i);
			if (show_dev_name.checked)
			{
				strcat(#disc_name, #dev_name);
				if (is_active) WriteText(draw_x+30,draw_y+5,0x80,0x555555,#disc_name);
				WriteText(draw_x+29,draw_y+5,0x80,0,#disc_name);
				//if (is_active) kfont.bold = true;
				//kfont.WriteIntoWindow(draw_x + 29, draw_y+2, 0xFFFfff, 0x000000, kfont.size.pt, #disc_name);
				//kfont.bold = false;
			} else {
				if (is_active) WriteText(draw_x+30,draw_y+5,0x80,0x555555,#dev_name);
				WriteText(draw_x+29,draw_y+5,0x80,0,#dev_name);
			}
			_PutImage(draw_x+6,draw_y, 18,17, is_active*7+dev_icon*17*18*3+#devices);
			draw_y += DEV_H;			
		}
		DrawBar(draw_x+6, draw_y, 18, 1, 0xFFFfff);
	}
}

void _SystemDiscs::DrawSelect(int draw_x, btid, dword _path)
{
	#define SELECTY 10
	char dev_name[15], disc_name[100], i, dev_icon;

	if (ESBYTE[_path+1]=='\0') {
		strcpy(#dev_name, "/root");
		dev_icon = 0;
	} else if (chrnum(_path, '/')==1) {
		strcpy(#dev_name, _path);
		GetDiskIconAndName(dev_name[1], #dev_icon, #disc_name);
	} else for (i=0;i<list.count;i++) {
		strcpy(#dev_name, list.get(i));
		GetDiskIconAndName(dev_name[1], #dev_icon, #disc_name);
		if (strstr(_path, #dev_name)-_path==0) break;
	}
	DrawRectangle(draw_x-1, SELECTY-1, DDW+2-KFM2_DEVH, KFM2_DEVH+1, sc.work_graph);
	DrawBar(draw_x, SELECTY, DDW+1-KFM2_DEVH, KFM2_DEVH, 0xFFFFFF);
	_PutImage(draw_x + 5, SELECTY+2, 18,17, dev_icon*17*18*3+#devices);
	kfont.WriteIntoWindow(draw_x + 24, math.max(KFM2_DEVH-kfont.height/2+SELECTY,0), 0xFFFfff, 0x000000, kfont.size.pt, #dev_name+1);
	DefineHiddenButton(draw_x, SELECTY, DDW-1, KFM2_DEVH-1, btid+1);
	DrawFlatButtonSmall(draw_x+DDW-KFM2_DEVH+1, SELECTY-1, KFM2_DEVH-1, KFM2_DEVH+1, btid, "\x19");
}

void _SystemDiscs::DrawOptions(int draw_x)
{
	int optionsy = SELECTY+KFM2_DEVH+1;
	char dev_name[15], disc_name[100], i, dev_icon, is_active=0;
		
	for (i=0; i<30; i++) DeleteButton(100+i);

	DrawPopup(draw_x, optionsy, DDW, list.count*KFM2_DEVH, 1, -1, sc.work_graph);

	for (i=0;i<list.count;i++) {
		strcpy(#dev_name, list.get(i));
		GetDiskIconAndName(dev_name[1], #dev_icon, #disc_name);
		if (strstr(path, #dev_name)!=0) is_active=true; else is_active=false;

		DrawBar(draw_x, optionsy, DDW, KFM2_DEVH, 0xFFFFFF);
		DefineButton(draw_x, optionsy, DDW, KFM2_DEVH-1, 100+i+BT_HIDE,0xFFFFFF);
		_PutImage(draw_x + 5, optionsy+2, 18,17, is_active*7+dev_icon*17*18*3+#devices);
		if (is_active) kfont.bold = true;
		kfont.WriteIntoWindow(draw_x + 24, optionsy+2, 0xFFFfff, 0x000000, kfont.size.pt, #dev_name+1);
		kfont.bold = false;
		optionsy += KFM2_DEVH;
	}
}

void _SystemDiscs::Click(int n)
{
	strcpy(path, list.get(n));
	files.KeyHome();
	Open_Dir(path,WITH_REDRAW);	
}



void DrawDeviceAndActionsLeftPanel()
{
	Tip(56, T_DEVICES, 55, "=");
	SystemDiscs.Draw();
	ActionsDraw();
	DrawLeftPanelBg();
}

dword col_palette_inner[14] = {0xD2D3D3,0xD4D4D4,0xD6D5D6,0xD8D7D8,0xDAD8D9,0xDCDADB,
	0xDFDCDD,0xE1DDDE,0xE2DEE0,0xE4DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1};
void DrawFilledBarInner(dword x, y, w, h)
{
	int i, fill_h;
	if (h <= 14) fill_h = h; else fill_h = 14;
	for (i=0; i<fill_h; i++) DrawBar(x, y+i, w, 1, col_palette_inner[14-i]);
	DrawBar(x, y+i, w, h-fill_h, col_palette_inner[14-i]);
}

void Tip(int y, dword caption, id, arrow)
{
	if (col.def) {
		DrawBar(17,y,160,1,0xEFEDEE);
		DrawFilledBarInner(17, y+1, 160, 16);
		DrawBar(17,y+17,160,1,0x7E87A3);
	} else {
		DrawBar(17,y,160,1,sc.work_graph);
		DrawBar(17,y+1,160,16,col.list_bg);
		DrawBar(17,y+17,160,1,sc.work_graph);
	}
	WriteText(25,y+5,0x80,col.list_gb_text,caption);
	if (id) DefineButton(159,y+1,16,16,id+BT_HIDE+BT_NOFRAME,0); //arrow button
	WriteText(165,y+5,0x80,col.list_gb_text,arrow); //arrow
}

void ActionsDraw()
{
	int i;
	int actions_y= SystemDiscs.list.count*DEV_H+108;
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

void DrawLeftPanelBg()
{
	int actions_y = SystemDiscs.list.count*DEV_H;
	int start_y = actions_y+159;
	int area_h;
	DrawBar(2,41,190,15,waves_pal[0]);             //above devices block
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
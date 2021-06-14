
#ifdef LANG_RUS
	?define T_DEVICES "Устройства"
	?define T_ACTIONS "Действия"
	char *actions[] = {
		59, "Новый файл", "F9",
		57, "Новая папка", "F7",
		60, "Настройки", "F10",
		0,0,0
	};
	?define T_PROG "Программы "
	?define T_SYS  "Система "
	?define T_UNC  "Неизвестно "
	?define T_CD   "CD-ROM "
	?define T_FD   "Дискета "
	?define T_HD   "Жесткий диск "
	?define T_SATA "SATA диск "
	?define T_USB  "USB диск "
	?define T_RAM  "RAM диск "
#else
	?define T_DEVICES "Devices"
	?define T_ACTIONS "Actions"
	char *actions[] = {
		59, "New file", "F9",
		57, "New folder", "F7",
		60, "Settings", "F10",
		0,0,0
	};
	?define T_PROG "Programs "
	?define T_SYS  "System "
	?define T_UNC  "Unknown "
	?define T_CD   "CD-ROM "
	?define T_FD   "Floppy disk "
	?define T_HD   "Hard disk "
	?define T_SATA "SATA disk "
	?define T_USB  "USB disk "
	?define T_RAM  "RAM disk "
#endif

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

#define DEV_H 17
#define DEV_H_HOR 20

void GetDiskIconAndName(char disk_first_letter, dword dev_icon, disc_name)
{
	switch(disk_first_letter)
	{
		case 'r':
			ESBYTE[dev_icon]=0;
			strcpy(disc_name, T_SYS);
			break;
		case 'k':
			ESBYTE[dev_icon]=1;
			strcpy(disc_name, T_PROG);
			break;
		case 'f':
			ESBYTE[dev_icon]=2;
			strcpy(disc_name, T_FD);
			break;
		case 'c':
			ESBYTE[dev_icon]=3;
			strcpy(disc_name, T_CD);
			break;
		case 'h':
		case 'b':
			ESBYTE[dev_icon]=4;
			strcpy(disc_name, T_HD);
			break;
		case 's':
			ESBYTE[dev_icon]=4;
			strcpy(disc_name, T_SATA);
			break;
		case 't':
			ESBYTE[dev_icon]=5;
			strcpy(disc_name, T_RAM);
			break;
		case 'u':
			ESBYTE[dev_icon]=6;
			strcpy(disc_name, T_USB);
			break;
		default:
			ESBYTE[dev_icon]=5;
	}
}

void _SystemDiscs::Get()
{
	bool kolibrios_exists=false;
	char dev_name[10], sys_discs[10];
	int i1, j1, dev_num_i, dev_disc_num;
	dword devbuf;

	list.drop();
	devbuf = malloc(10000);
	ReadDir(19, devbuf, "/");
	dev_num = dev_num_i = EBX;
	for (i1=0; i1<dev_num_i; i1++)
	{
		sprintf(#dev_name,"/%s",i1*304+ devbuf+72);
		Open_Dir(#dev_name, ONLY_OPEN);
		dev_disc_num = files.count;
		for (j1=0; j1<dev_disc_num; j1++;)
		{
			sprintf(#sys_discs,"%s/%s",#dev_name,j1*304+ buf+72);
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
}

#define DDW 120

void _SystemDiscs::Draw()
{    
	char dev_name[15], disc_name[100], i, dev_icon, is_active=0, name_len;
	int draw_y, draw_x;
	
	for (i=0; i<30; i++) DeleteButton(100+i);

	if (efm) { 
		if (active_panel==1) {
			DrawSelect(Form.cwidth/2-DDW, 10, #path, KFM_DEV_DROPDOWN_1);
			DrawSelect(Form.cwidth-DDW-2, 10, #inactive_path, KFM_DEV_DROPDOWN_2);
		} else {
			DrawSelect(Form.cwidth/2-DDW, 10, #inactive_path, KFM_DEV_DROPDOWN_1);
			DrawSelect(Form.cwidth-DDW-2, 10, #path, KFM_DEV_DROPDOWN_2);		
		}
		files.y = 40 + 17;
	} else { 
		draw_y = 74; 
		draw_x = 17; 
		for (i=0;i<list.count;i++) {
			strcpy(#dev_name, list.get(i));
			GetDiskIconAndName(dev_name[1], #dev_icon, #disc_name);
			if (strstr(#path, #dev_name)!=0) is_active=true; else is_active=false;

			DrawBar(draw_x,draw_y,6,DEV_H+1,0xFFFFFF);
			DrawBar(draw_x+6+18,draw_y,160-6-18,DEV_H+1,0xFFFFFF);
			DefineButton(draw_x,draw_y,159,16,100+i+BT_HIDE,0xFFFFFF);
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

void _SystemDiscs::DrawSelect(int draw_x, draw_y, path1, btid)
{
	char dev_name[15], disc_name[100], i, dev_icon, is_active=0;
	if (ESBYTE[path1+1]=='\0') {
		strcpy(#dev_name, "/root");
		dev_icon = 0;
	} else if (chrnum(path1, '/')==1) {
		strcpy(#dev_name, path1);
		GetDiskIconAndName(dev_name[1], #dev_icon, #disc_name);
	} else for (i=0;i<list.count;i++) {
		strcpy(#dev_name, list.get(i));
		GetDiskIconAndName(dev_name[1], #dev_icon, #disc_name);
		if (strstr(path1, #dev_name)-path1==0) break;
	}
	DrawRectangle(draw_x-1, draw_y-1, DDW+2-DEV_H_HOR, DEV_H_HOR+1, sc.work_graph);
	DrawBar(draw_x, draw_y, DDW+1-DEV_H_HOR, DEV_H_HOR, 0xFFFFFF);
	_PutImage(draw_x + 5, draw_y+2, 18,17, is_active*7+dev_icon*17*18*3+#devices);
	kfont.WriteIntoWindow(draw_x + 24, draw_y+2, 0xFFFfff, 0x000000, kfont.size.pt, #dev_name+1);
	UnsafeDefineButton(draw_x, draw_y, DDW-1, DEV_H_HOR-1, btid+1+BT_HIDE,0xFFFFFF);
	DrawFlatButtonSmall(draw_x+DDW-DEV_H_HOR+1, draw_y-1, DEV_H_HOR-1, DEV_H_HOR+1, btid, "\x19");

	draw_x += DDW + 1;
	//DrawBar(draw_x, draw_y, Form.cwidth - draw_x - 2, DEV_H_HOR, sc.work);
}

void _SystemDiscs::DrawOptions(int draw_x, draw_y)
{
	char dev_name[15], disc_name[100], i, dev_icon, is_active=0;
		
	for (i=0; i<30; i++) DeleteButton(100+i);

	DrawPopup(draw_x, draw_y, DDW, list.count*DEV_H_HOR, 1, -1, sc.work_graph);

	for (i=0;i<list.count;i++) {
		strcpy(#dev_name, list.get(i));
		GetDiskIconAndName(dev_name[1], #dev_icon, #disc_name);
		if (strstr(#path, #dev_name)!=0) is_active=true; else is_active=false;

		DrawBar(draw_x, draw_y, DDW, DEV_H_HOR, 0xFFFFFF);
		DefineButton(draw_x, draw_y, DDW, DEV_H_HOR-1, 100+i+BT_HIDE,0xFFFFFF);
		_PutImage(draw_x + 5, draw_y+2, 18,17, is_active*7+dev_icon*17*18*3+#devices);
		if (is_active) kfont.bold = true;
		kfont.WriteIntoWindow(draw_x + 24, draw_y+2, 0xFFFfff, 0x000000, kfont.size.pt, #dev_name+1);
		kfont.bold = false;
		draw_y += DEV_H_HOR;
	}
}

void _SystemDiscs::Click(int n)
{
	strcpy(#path, list.get(n));
	files.KeyHome();
	Open_Dir(#path,WITH_REDRAW);	
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
		DrawBar(17,actions_y,160,DEV_H,0xFFFFFF); //схыюх
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
	int i;
	DrawBar(2,41,190,15,waves_pal[0]);		      //above devices block
	DrawBar(17,actions_y+75,160,15,waves_pal[0]); //below devices block
	PutShadow(17,actions_y+75,160,1,1,3);
	PutShadow(18,actions_y+75+1,158,1,1,1);
	DrawBar(2,56,15,actions_y+103,waves_pal[0]);	          //on the left
	DrawBar(177,56,15,actions_y+103,waves_pal[0]);            //on the right
	area_h = Form.cheight-start_y-2 - status_bar_h;
	if (area_h < 268){
		PutPaletteImage(#blue_hl, 190, area_h, 2, start_y, 8, #waves_pal);
	} else {
		DrawBar(2,start_y,190, area_h-268, waves_pal[0]);
		PutPaletteImage(#blue_hl, 190, 268, 2, Form.cheight-270-status_bar_h, 8, #waves_pal);
	}
	PutShadow(17,start_y,160,1,1,3);
	PutShadow(18,start_y+1,158,1,1,1);
}
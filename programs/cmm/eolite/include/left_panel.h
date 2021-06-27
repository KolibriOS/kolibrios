
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

void _SystemDiscs::Draw()
{    
	char dev_name[15], disc_name[100], i, dev_icon;
	bool is_active=0;
	int draw_y, draw_x;
	
	if (efm) { 
		DrawSelect(2, KFM_DEV_DROPDOWN_1, location[0]);
		DrawSelect(Form.cwidth/2, KFM_DEV_DROPDOWN_2, location[sizeof(dword)]);
		files.y = 40 + 17;
	} else { 
		Tip(56, T_DEVICES, 55, "=");
		for (i=0; i<30; i++) DeleteButton(100+i);

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
		ActionsDraw(list.count*DEV_H+108);
		DrawLeftPanelBg(list.count*DEV_H);
	}
}

void _SystemDiscs::DrawSelect(int draw_x, btid, dword _path)
{
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


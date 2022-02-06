bool KolibriosMounted()
{
	static bool kolibrios_mounted;
	if (kolibrios_mounted) return true;
	kolibrios_mounted = real_dir_exists("/kolibrios");
	return kolibrios_mounted;
}

struct _SystemDiscs
{
	collection list;
	int dev_num;
	void Get();
	void Draw();
	void DrawSelect();
	void DrawOptions();
} SystemDiscs=0;

void _SystemDiscs::Get()
{
	char dev_name[10], sys_discs[10];
	int i1, j1, dev_disc_num, real_dev_num;
	dword devbuf, diskbuf;

	list.drop();
	devbuf = malloc(10000);
	ReadDir(19, devbuf, "/");
	dev_num = real_dev_num = EBX;

	list.add("/sys");
	dev_num++;

	if (KolibriosMounted()) {
		//need to check that /sys != /kolibrios
		list.add("/kolibrios");
		dev_num++;
	}

	for (i1=0; i1<real_dev_num; i1++)
	{
		sprintf(#dev_name,"/%s",i1*304+ devbuf+72);
		GetDir(#diskbuf, #dev_disc_num, #dev_name, DIRS_NOROOT);
		if (!EAX) for (j1=0; j1<dev_disc_num; j1++;)
		{
			sprintf(#sys_discs,"%s/%s",#dev_name,j1*304+ diskbuf+72);
			if (sys_discs[1]=='r') {
				dev_num--;
				continue;
			}
			if (sys_discs[1]=='c') || (sys_discs[1]=='f') || (dir_exists(#sys_discs)) {
				list.add(#sys_discs);
			}
		}
	}
	free(devbuf);
	free(diskbuf);
}

void GetDiskIconAndName(dword dev_name, icon, disc_name)
{
	int i;
	dword volume_label;
	for (i=0; devinfo[i]!=0; i+=3) {
		if (!strncmp(dev_name+1, devinfo[i], 2)) {
			ESBYTE[icon] = devinfo[i+1];
			if (ESBYTE[icon]==4)
			{
				//show label only for hard disk drives
				volume_label = GetVolumeLabel(dev_name);
				if (ESBYTE[volume_label]) {
					strncpy(disc_name, volume_label, 15);
					chrcat(disc_name, ' ');
					return;					
				}
			}
			strcpy(disc_name, devinfo[i+2]);
			return;
		}
	}
	ESBYTE[icon]=4;
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
			GetDiskIconAndName(#dev_name, #dev_icon, #disc_name);
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
			PutImage(draw_x+6,draw_y, 18,17, is_active*7+dev_icon*17*18*3+#devices);
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
		GetDiskIconAndName(#dev_name, #dev_icon, #disc_name);
	} else for (i=0;i<list.count;i++) {
		strcpy(#dev_name, list.get(i));
		GetDiskIconAndName(#dev_name, #dev_icon, #disc_name);
		if (strstr(_path, #dev_name)-_path==0) break;
	}
	DrawRectangle(draw_x-1, SELECTY-1, DDW+2-KFM2_DEVH, KFM2_DEVH+1, sc.line);
	DrawBar(draw_x, SELECTY, DDW+1-KFM2_DEVH, KFM2_DEVH, 0xFFFFFF);
	PutImage(draw_x + 5, SELECTY+2, 18,17, dev_icon*17*18*3+#devices);
	kfont.WriteIntoWindow(draw_x + 24, math.max(KFM2_DEVH-kfont.height/2+SELECTY,0), 0xFFFfff, 0x000000, kfont.size.pt, #dev_name+1);
	DefineHiddenButton(draw_x, SELECTY, DDW-1, KFM2_DEVH-1, btid);
	DrawFlatButtonSmall(draw_x+DDW-KFM2_DEVH+1, SELECTY-1, KFM2_DEVH-1, KFM2_DEVH+1, NULL, "\x19");
}

void _SystemDiscs::DrawOptions(int draw_x)
{
	int optionsy = SELECTY+KFM2_DEVH+1;
	char dev_name[15], disc_name[100], volume_label[64], label_file_name[100];
	char i, dev_icon, is_active=0;
		
	SystemDiscs.Get();

	DrawPopup(draw_x, optionsy, DDW, list.count*KFM2_DEVH, 1, -1, sc.line);

	for (i=0;i<list.count;i++) {
		strcpy(#dev_name, list.get(i));
		GetDiskIconAndName(#dev_name, #dev_icon, #disc_name);
		if (strstr(path, #dev_name)!=0) is_active=true; else is_active=false;

		DrawBar(draw_x, optionsy, DDW, KFM2_DEVH, 0xFFFFFF);
		DefineButton(draw_x, optionsy, DDW, KFM2_DEVH-1, 100+i+BT_HIDE,0xFFFFFF);
		PutImage(draw_x + 5, optionsy+2, 18,17, is_active*7+dev_icon*17*18*3+#devices);
		if (is_active) kfont.bold = true;
		//strncpy(#volume_label, GetVolumeLabel(#dev_name), sizeof(volume_label));
		strcpy(#label_file_name, #dev_name);
		//if (dev_name[1]!='k') && (dev_name[2]!='y') {
		//	if (volume_label) sprintf(#label_file_name, "%s [%s]", #dev_name, #volume_label);
		//} 
		kfont.WriteIntoWindow(draw_x + 24, optionsy+2, 0xFFFfff, 0x000000, kfont.size.pt, #label_file_name+1);
		kfont.bold = false;
		optionsy += KFM2_DEVH;
	}
}



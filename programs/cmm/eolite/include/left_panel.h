
#ifdef LANG_RUS
	?define T_DEVICES "УбваЃ©бвҐ†"
	?define T_ACTIONS "Д•©бвҐ®п"
	char *actions[] = {
		57, "НЃҐл© д†©Ђ", "F7",
		56, "НЃҐ†п ѓ†ѓ™†", "F6",
		60, "Н†бваЃ©™®", "F10",
		0,0,0
	};
	?define T_PROG "ПаЃ£а†ђђл "
	?define T_SYS  "С®бв•ђ† "
	?define T_UNC  "Н•®ІҐ•бв≠Ѓ "
	?define T_CD   "CD-ROM "
	?define T_FD   "Д®б™•в† "
	?define T_HD   "Ж•бв™®© §®б™ "
	?define T_SATA "SATA §®б™ "
	?define T_USB  "USB §®б™ "
	?define T_RAM  "RAM §®б™ "
#else
	?define T_DEVICES "Devices"
	?define T_ACTIONS "Actions"
	char *actions[] = {
		57, "New file", "F7",
		56, "New folder", "F6",
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
	void Click();
} SystemDiscs;


void GetDiskIconAndName(char disk_first_letter, dword dev_icon, disc_name)
{
	switch(disk_first_letter)
	{
		case 'k':
			ESBYTE[dev_icon]=0;
			strcpy(disc_name, T_PROG);
			break;
		case 'r':
			ESBYTE[dev_icon]=0;
			strcpy(disc_name, T_SYS);
			break;
		case 'c':
			ESBYTE[dev_icon]=1;
			strcpy(disc_name, T_CD);
			break;
		case 'f':
			ESBYTE[dev_icon]=2;
			strcpy(disc_name, T_FD);
			break;
		case 'h':
		case 'b':
			ESBYTE[dev_icon]=3;
			strcpy(disc_name, T_HD);
			break;
		case 's':
			ESBYTE[dev_icon]=3;
			strcpy(disc_name, T_SATA);
			break;
		case 'u':
			ESBYTE[dev_icon]=5;
			strcpy(disc_name, T_USB);
			break;
		case 't':
			ESBYTE[dev_icon]=4;
			strcpy(disc_name, T_RAM);
			break;
		default:
			ESBYTE[dev_icon]=3;
			strcpy(disc_name, T_UNC);				
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
			list.add(#sys_discs);
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

void _SystemDiscs::Draw()
{    
	char dev_name[15], disc_name[100], i, dev_icon, is_active, name_len;
	int draw_y, draw_x, draw_h;
	
	for (i=0; i<30; i++) DeleteButton(100+i);

	if ( two_panels.checked) { draw_y = 41; draw_x =  2; draw_h = 21; }
	if (!two_panels.checked) { draw_y = 74; draw_x = 17; draw_h = 16; }

	for (i=0;i<list.count;i++)
	{
		strcpy(#dev_name, list.get(i));
		GetDiskIconAndName(dev_name[1], #dev_icon, #disc_name);
		if (strstr(#path, #dev_name)-#path==0) is_active=true; else is_active=false;
		if (two_panels.checked)
		{
			name_len = strlen(#dev_name)-1*8;
			DrawBar(draw_x, draw_y, name_len + 31, draw_h, 0xFFFFFF);
			DefineButton(draw_x+2, draw_y, name_len + 27, draw_h-1, 100+i+BT_HIDE,0xFFFFFF);
			_PutImage(draw_x + 5, draw_y+2, 18,17, is_active*6+dev_icon*17*18*3+#devices);
			WriteText(draw_x + 24, draw_y+3, 10110000b, 0, #dev_name+1);
			draw_x += name_len + 31;
			if (draw_x>=Form.width-35) && (Form.width) {
				DrawBar(draw_x, draw_y, Form.cwidth - draw_x - 2, draw_h, 0xFFFFFF);
				draw_x = 2;
				draw_y += draw_h;
			}
		}
		else
		{
			DrawBar(draw_x,draw_y,6,draw_h+1,0xFFFFFF);
			DrawBar(draw_x+6+18,draw_y,160-6-18,draw_h+1,0xFFFFFF);
			DefineButton(draw_x,draw_y,159,16,100+i+BT_HIDE,0xFFFFFF);
			if (show_dev_name.checked)
			{
				strcat(#disc_name, #dev_name);
				if (is_active) WriteText(draw_x+30,draw_y+5,0x80,0x555555,#disc_name);
				WriteText(draw_x+29,draw_y+5,0x80,0,#disc_name);			}
			else
			{
				if (is_active) WriteText(draw_x+30,draw_y+5,0x80,0x555555,#dev_name);
				WriteText(draw_x+29,draw_y+5,0x80,0,#dev_name);
			}
			_PutImage(draw_x+6,draw_y, 18,17, is_active*6+dev_icon*17*18*3+#devices);
			draw_y += draw_h;
		}
	}
	if (two_panels.checked)
	{
		DrawBar(draw_x, draw_y, Form.cwidth - draw_x - 2, draw_h, 0xFFFFFF);
		DefineButton(Form.cwidth - 23, draw_y+2, 17,16, 60+BT_HIDE, 0xCCCccc);
		_PutImage(Form.cwidth - 21, draw_y+4, 14,13, 2*14*13*3+#factions);
		files.y = draw_y + draw_h + 17;
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
	DrawBar(17,y,160,1,0xEFEDEE);
	DrawFilledBarInner(17, y+1, 160, 16);
	WriteText(25,y+5,0x80,0x000000,caption);
	if (id) DefineButton(159,y+1,16,16,id+BT_HIDE+BT_NOFRAME,0); //arrow button
	WriteText(165,y+5,0x80,0x000000,arrow); //arrow
	DrawBar(17,y+17,160,1,0x7E87A3);
}

void ActionsDraw()
{
	int i;
	int actions_y= SystemDiscs.list.count*16+108, lineh=16;
	Tip(actions_y-18, T_ACTIONS, 77, ""); //заголовок
	for (i=0; actions[i*3]!=0; i++, actions_y+=lineh)
	{
		DrawBar(17,actions_y,160,lineh,0xFFFFFF); //белое
		DefineButton(17,actions_y,159,lineh,actions[i*3]+BT_HIDE,0xE4DFE1);
		WriteText(45,actions_y+4,0x80,0,actions[i*3+1]);
		WriteText(-strlen(actions[i*3+2])*6+170,actions_y+4,0x80,0x999999,actions[i*3+2]);
		_PutImage(23,actions_y+2, 14,13, i*14*13*3+#factions);
	}
}

void DrawLeftPanelBg()
{
	int actions_y = SystemDiscs.list.count*16;
	int start_y = actions_y+156;
	int area_h;
	DrawBar(2,41,190,15,col_lpanel);		      //синий пр€моугольник - над девайсами
	DrawBar(17,actions_y+75,160,15,col_lpanel); //синий пр€моугольник - под девайсами
	PutShadow(17,actions_y+75,160,1,1,3);
	PutShadow(18,actions_y+75+1,158,1,1,1);
	DrawBar(2,56,15,actions_y+103,col_lpanel);	          //синий пр€моугольник - слева       
	DrawBar(177,56,15,actions_y+103,col_lpanel);            //синий пр€моугольник - справа
	area_h = Form.cheight-start_y-2 - status_bar_h;
	if (area_h < 268)
	{
		PutPaletteImage(#blue_hl, 190, area_h, 2, start_y, 8, #blue_hl_pal);
	}
	else
	{
		DrawBar(2,start_y,190, area_h-268, col_lpanel);
		PutPaletteImage(#blue_hl, 190, 268, 2, Form.cheight-270-status_bar_h, 8, #blue_hl_pal);
	}
	PutShadow(17,start_y,160,1,1,3);
	PutShadow(18,start_y+1,158,1,1,1);
}
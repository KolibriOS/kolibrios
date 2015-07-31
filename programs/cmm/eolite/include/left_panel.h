//Leency 2008-2013

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
#elif LANG_EST
	?define T_DEVICES "Seadmed"
	?define T_ACTIONS "Toimingud"
	char *actions[] = {
		57, "Uus fail", "F7",
		56, "Uus kataloog", "F6",
		60, "Seaded", "F10",
		0,0,0
	};
	?define T_PROG "Programs "
	?define T_SYS  "System "
	?define T_UNC  "Unknown "
	?define T_CD   "CD-ROM "
	?define T_FD   "Floppy disk "
	?define T_HD   "Hard disk "
	?define T_SATA "SATA disk"
	?define T_USB  "USB disk"
	?define T_RAM  "RAM disk"
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
	?define T_SATA "SATA disk"
	?define T_USB  "USB disk"
	?define T_RAM  "RAM disk"
#endif


void Tip(int y, dword caption, id, arrow)
{
	int i;
	DrawBar(17,y,160,1,0xEFEDEE);
	DrawFilledBar(17, y+1, 160, 16);
	WriteText(25,y+5,0x80,sc.work_text,caption);
	if (id) DefineButton(159,y+1,16,16,id+BT_HIDE+BT_NOFRAME,0); //arrow button
	WriteText(165,y+5,0x80,sc.work_text,arrow); //arrow
	DrawBar(17,y+17,160,1,sc.work_graph);
}


path_string disk_list[30];
int disc_num;
dword devbuf;


void GetSystemDiscs()
{
	char dev_name[10], sys_discs[10];
	int i1, j1, dev_num, dev_disc_num,l;
	disc_num=0;
	if (devbuf) free(devbuf);
	devbuf = malloc(10000); //буфер где-то на 10 девайсов в левой панели
	ReadDir(19, devbuf, "/");
	dev_num = EBX;
	for (i1=0; i1<dev_num; i1++)
	{
		sprintf(#dev_name,"/%s/",i1*304+ devbuf+72);
		Open_Dir(#dev_name, ONLY_OPEN);
		dev_disc_num = files.count;
		//if (files.count<=0) copystr(#dev_name,#disk_list[disc_num].Item); else
		for (j1=0; j1<dev_disc_num; j1++;)
		{
			l=sprintf(#sys_discs,"%s%s/",#dev_name,j1*304+ buf+72);
			strncpy(#disk_list[disc_num].Item, #sys_discs,l);
			disc_num++;
		}
		if (!strncmp(#sys_discs, "/rd/1/",6)) 
		{
			if (isdir("/kolibrios"))
			{
				strncpy(#disk_list[disc_num].Item, "/kolibrios/",11);
				kolibrios_drive = true;
				disc_num++;	
			}
			else kolibrios_drive = false;
		}
	}
}


void DrawSystemDiscs()
{    
	char dev_name[15], disc_name[100], i, dev_icon, is_active;
	int pos_y;
	
	for (i=disc_num; i<30; i++) DeleteButton(100+i);
	for (i=0;i<disc_num;i++)
	{
		pos_y = i*16+74;
		DrawBar(17,pos_y,6,17,0xFFFFFF);
		DrawBar(17+6+18,pos_y,160-6-18,17,0xFFFFFF);
		DefineButton(17,pos_y,159,16,100+i+BT_HIDE,0xFFFFFF);
		strcpy(#dev_name, #disk_list[i].Item);
		dev_name[strlen(#dev_name)-1]=NULL;
		switch(dev_name[1])
		{
			case 'k':
				dev_icon=0;
				strcpy(#disc_name, T_PROG);
				break;
			case 'r':
				dev_icon=0;
				strcpy(#disc_name, T_SYS);
				break;
			case 'c':
				dev_icon=1;
				strcpy(#disc_name, T_CD);
				break;
			case 'f':
				dev_icon=2;
				strcpy(#disc_name, T_FD);
				break;
			case 'h':
			case 'b':
				dev_icon=3;
				strcpy(#disc_name, T_HD);
				break;
			case 's':
				dev_icon=3;
				strcpy(#disc_name, T_SATA);
				break;
			case 'u':
				dev_icon=5;
				strcpy(#disc_name, T_USB);
				break;
			case 't':
				dev_icon=4;
				strcpy(#disc_name, T_RAM);
				break;
			default:
				dev_icon=3; //по-умолчанию устройство выгл€дит как жест€к но это неправильно
				strcpy(#disc_name, T_UNC);				
		}
		if (strstr(#path, #dev_name)) is_active=true; else is_active=false;
		if (show_dev_name)
		{
			strcat(#disc_name, #dev_name);
			if (is_active) WriteText(46+1,pos_y+5,0x80,0x555555,#disc_name);
			WriteText(46,pos_y+5,0x80,0,#disc_name);
		}
		else
		{
			if (is_active) WriteText(46+1,pos_y+5,0x80,0x555555,#dev_name);
			WriteText(46,pos_y+5,0x80,0,#dev_name);
		}
		_PutImage(23,pos_y, 18,17, is_active*6+dev_icon*17*18*3+#devices);
	}
}

void ActionsDraw()
{
	int actions_y=disc_num*16+108, lineh=16;
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
	int actions_y=disc_num*16;
	int start_y = actions_y+156;
	DrawBar(2,41,190,15,col_lpanel);		      //синий пр€моугольник - над девайсами
	DrawBar(17,actions_y+75,160,15,col_lpanel); //синий пр€моугольник - под девайсами
	PutShadow(17,actions_y+75,160,1,1,3);
	PutShadow(18,actions_y+75+1,158,1,1,1);
	DrawBar(2,56,15,actions_y+103,col_lpanel);	          //синий пр€моугольник - слева       
	DrawBar(177,56,15,actions_y+103,col_lpanel);            //синий пр€моугольник - справа
	if (onTop(start_y, 6) < 268)
		PutPaletteImage(#blue_hl, 190, onTop(start_y, 6), 2, start_y, 8, #blue_hl_pal);
	else
	{
		DrawBar(2,start_y,190,onTop(start_y,6+268),col_lpanel);
		PutPaletteImage(#blue_hl, 190, 268, 2, onTop(268,6), 8, #blue_hl_pal);
	}
	PutShadow(17,start_y,160,1,1,3);
	PutShadow(18,start_y+1,158,1,1,1);
}


void DrawLeftPanel()
{
	Tip(56, T_DEVICES, 55, "=");
	DrawSystemDiscs();
	ActionsDraw();
	DrawLeftPanelBg();
}

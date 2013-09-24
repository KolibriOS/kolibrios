//Leency 2008-2013

#ifdef LANG_RUS
char *actions[] = {
	57, "НЃҐл© д†©Ђ", "F7",
	56, "НЃҐ†п ѓ†ѓ™†", "F6",
	60, "Н†бваЃ©™®", "F10",
	0,0,0
};
?define T_DEVICES "УбваЃ©бвҐ†"
?define T_ACTIONS "Д•©бвҐ®п"

#elif LANG_EST
char *actions[] = {
	57, "Uus fail", "F7",
	56, "Uus kataloog", "F6",
	60, "Seaded", "F10",
	0,0,0
};

?define T_DEVICES "Seadmed"
?define T_ACTIONS "Toimingud"

#else
char *actions[] = {
	57, "New file", "F7",
	56, "New folder", "F6",
	60, "Options", "F10",
	0,0,0
};

?define T_DEVICES "Devices"
?define T_ACTIONS "Actions"
#endif


void Tip(int y, dword caption, id, arrow)
{
	int i;
	DrawBar(17,y,160,1,0xEFEDEE);
	DrawFilledBar(17, y+1, 160, 16);
	WriteText(25,y+5,0x80,0,caption);
	IF (id<>0) DefineButton(159,y+1,16,16,id+BT_HIDE+BT_NOFRAME,0); //кнопа дл€ стрелки
	WriteText(165,y+5,0x80,0,arrow); //стрелка вниз
	DrawBar(17,y+17,160,1,col_border);		//подчЄркивание
}


path_string disk_list[30];
int disc_num;
dword devbuf;

void SystemDiscsGet()
{
	unsigned char dev_name[10], sys_discs[10];
	unsigned int i1, j1, dev_num, dev_disc_num;
	
	disc_num=0;
	if (devbuf) free(devbuf);
	devbuf = malloc(10000); //буфер где-то на 10 девайсов в левой панели
	ReadDir(19, devbuf, "/");
	dev_num = EBX;
	for (i1=0; i1<dev_num; i1++)
	{
		strcpy(#dev_name, "/");                                 // /
		strcat(#dev_name, i1*304+ devbuf+72); // /rd
		strcat(#dev_name, "/");               // /rd/
		Open_Dir(#dev_name, ONLY_OPEN);
		dev_disc_num = files.count;
		//if (files.count<=0) copystr(#dev_name,#disk_list[disc_num].Item); else
		for (j1=0; j1<dev_disc_num; j1++;)
		{
			strcpy(#sys_discs, #dev_name);                              // /rd/
			strcat(#sys_discs, j1*304+ buf+72);      // /rd/1
			strcat(#sys_discs, "/");                 // /rd/1/
			strcpy(#disk_list[disc_num].Item, #sys_discs);
			disc_num++;
		}
	}
}


void SystemDiscsDraw()
{    
	char dev_name[10], disc_name[100];
	int i, dev_icon;
	
	Tip(56, T_DEVICES, 55, "=");
	for (i=0; i<20; i++) DeleteButton(100+i);
	for (i=0;i<disc_num;i++)
	{
		DrawBar(17,i*16+74,160,17,0xFFFFFF);
		DefineButton(17,i*16+74,159,16,100+i+BT_HIDE,0xFFFFFF);
		strcpy(#dev_name, #disk_list[i].Item);
		dev_name[strlen(#dev_name)-1]=NULL;
		switch(dev_name[1])
		{
			case 'r':
				dev_icon=0;
				strcpy(#disc_name, "SYS disk ");
				break;
			case 'c':
				dev_icon=1;
				strcpy(#disc_name, "CD-ROM ");
				break;
			case 'f':
				dev_icon=2;
				strcpy(#disc_name, "Floppy disk ");
				break;
			case 'h':
			case 'b':
				dev_icon=3;
				strcpy(#disc_name, "Hard disk ");
				break;
			case 's':
				dev_icon=3;
				strcpy(#disc_name, "SATA disk ");
				break;
			case 'u':
				dev_icon=5;
				strcpy(#disc_name, "USB flash ");
				break;
			case 't':
				dev_icon=4;
				strcpy(#disc_name, "RAM disk ");				
				DefineButton(17+143,i*16+74,16,16,i+130+BT_HIDE+BT_NOFRAME,0xFFFFFF);
				WriteText(45+121,i*16+79,0x80,0xD63535,"-");
				WriteText(45+121,i*16+79+1,0x80,0xBC2424,"-");
				break;
			default:
				dev_icon=3; //по-умолчанию устройство выгл€дит как жест€к но это неправильно
				strcpy(#disc_name, "Unknown ");				
		}
		strcat(#disc_name, #dev_name);
		if (show_dev_name) WriteText(45,i*16+79,0x80,0,#disc_name);
			else WriteText(45,i*16+79,0x80,0,#dev_name);
		_PutImage(21,i*16+76, 14,13, dev_icon*14*13*3+#devices);
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
		_PutImage(21,actions_y+2, 14,13, i*14*13*3+#factions);
	}
}


void LeftPanelBgDraw()
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
	SystemDiscsDraw();
	ActionsDraw();
	LeftPanelBgDraw();
}


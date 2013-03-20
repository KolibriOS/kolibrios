//Leency 2008-2013

void Tip(int y, dword caption, id, arrow)
{
	int i;
	DrawBar(17,y,160,1,0xEFEDEE);
	DrawFilledBar(17, y+1, 160, 16);
	WriteText(25,y+5,0x80,0,caption);
	IF (id<>0) DefineButton(159,y+1,16,16,id+BT_HIDE+BT_NOFRAME,0xE4DFE1); //кнопа для стрелки
	WriteText(165,y+5,0x80,0,arrow); //стрелка вниз
	DrawBar(17,y+17,160,1,0x94AECE);		//подчёркивание
}


path_string disk_list[30];
int disc_num;
dword devbuf;

void GetSystemDiscs()
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
		dev_disc_num = count;
		//if (count<=0) copystr(#dev_name,#disk_list[disc_num].Item); else
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


void DrawSystemDiscs()
{    
	byte disc_icon;
	char dev_name[10];
	char disc_name[100];
	int i, dev_icon;
	
	for (i=0; i<20; i++) DeleteButton(100+i);
	//список дисков
	Tip(56, "Devices", 78, "=");
	for (i=0;i<disc_num;i++)
	{
		DrawBar(17,i*16+74,160,17,0xFFFFFF); //фон
		DefineButton(17,i*16+74,159,16,100+i+BT_HIDE,0xFFFFFF); //создаём кнопки, а потом выводим названия дисков
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
				dev_icon=3; //по-умолчанию устройство выглядит как жестяк но это неправильно
				strcpy(#disc_name, "Unknown ");				
		}
		strcat(#disc_name, #dev_name);
		if (show_dev_name) WriteText(45,i*16+79,0x80,0,#disc_name);
			else WriteText(45,i*16+79,0x80,0,#dev_name);
		_PutImage(21,i*16+76, 14,13, dev_icon*14*13*3+#devices);
	}
}

void FileMenu()
{
	word id, key;
	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				ExitProcess();
				break;
				
		case evKey:
				IF (GetKey()==27) ExitProcess();
				break;
				
		case evReDraw:
			DefineAndDrawWindow(m.x+1+Form.left,m.y+Form.top,159,90,0x01,0xEEEeee,0x01fffFFF);
			DrawBar(1,18,160,51,0xFFFFFF); //белое
			_PutImage(1,23, 16,44, #factions); //иконки
			//rename file 
			DefineButton(1,18,159,16,80+BT_HIDE,0xE4DFE1);
			WriteText(26,23,0x80,0,"Rename file");
			WriteText(134,23,0x80,0x999999,"[F2]");
			//delete file
			DefineButton(1,35,159,16,81+BT_HIDE,0xE4DFE1);
			WriteText(26,40,0x80,0,"Delete file");
			WriteText(144,40,0x80,0x999999,"[Del]");
			//create folder
			DefineButton(1,52,159,16,82+BT_HIDE,0xE4DFE1);
			WriteText(26,57,0x80,0,"Create folder");
			WriteText(134,57,0x80,0x999999,"[F6]");
	}
}

void Actions()
{
	int actions_y=disc_num*16;
	
	DeleteButton(80);
	DeleteButton(81);
	DeleteButton(82);
	
	if (!show_actions)
		Tip(actions_y+90, "Actions", 77, "\x18");
	else
	{
		Tip(actions_y+90, "Actions", 77, "\x19"); //заголовок
		DrawBar(17,actions_y+108,160,51,0xFFFFFF); //белое
		_PutImage(21,actions_y+113, 16,44, #factions); //иконки
		//rename file 
		DefineButton(17,actions_y+108,159,16,80+BT_HIDE,0xE4DFE1);
		WriteText(42,actions_y+113,0x80,0,"Rename file");
		WriteText(150,actions_y+113,0x80,0x999999,"[F2]");
		//delete file
		DefineButton(17,actions_y+125,159,16,81+BT_HIDE,0xE4DFE1);
		WriteText(42,actions_y+130,0x80,0,"Delete file");
		WriteText(144,actions_y+130,0x80,0x999999,"[Del]");
		//create folder
		DefineButton(17,actions_y+142,159,16,82+BT_HIDE,0xE4DFE1);
		WriteText(42,actions_y+147,0x80,0,"Create folder");
		WriteText(150,actions_y+147,0x80,0x999999,"[F6]");
	}
}


void LeftPanelBackground()
{
	int actions_y=disc_num*16;
	int start_y = show_actions*51+actions_y+108;
	DrawBar(2,41,190,15,col_lpanel);		      //синий прямоугольник - над девайсами
	DrawBar(17,actions_y+75,160,15,col_lpanel); //синий прямоугольник - под девайсами
	DrawBar(2,56,15,actions_y+103,col_lpanel);	          //синий прямоугольник - слева       
	DrawBar(177,56,15,actions_y+103,col_lpanel);            //синий прямоугольник - справа
	if (onTop(start_y, 6) < 268)
		PutPaletteImage(#blue_hl, 190, onTop(start_y, 6), 2, start_y, 8, #blue_hl_pal);
	else
	{
		DrawBar(2,start_y,190,onTop(start_y,6+268),col_lpanel);
		PutPaletteImage(#blue_hl, 190, 268, 2, onTop(268,6), 8, #blue_hl_pal);
	}
}


void DrawLeftPanel()
{
	DrawSystemDiscs();
	Actions();
	LeftPanelBackground();
}


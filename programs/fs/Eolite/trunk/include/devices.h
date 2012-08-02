//03.04.2012

path_string disk_list[20];
int disc_num;

void GetSystemDiscs()
{
	char dev_name[10], sys_discs[10];
	int i1, j1, dev_num, dev_disc_num;
	dword devbuf;

	disc_num=0;
	
	devbuf= malloc(3112); //буфер где-то на 10 девайсов в левой панели
	ReadDir(10, devbuf, "/");
	dev_num = EBX;
	for (i1=0; i1<dev_num; i1++)
	{
		copystr("/", #dev_name);                                 // /
		copystr(i1*304+ devbuf+72, #dev_name+strlen(#dev_name)); // /rd
		copystr("/", #dev_name+strlen(#dev_name));               // /rd/
		
		Open_Dir(#dev_name, ONLY_OPEN);
		dev_disc_num = count;
		//if (count<=0) copystr(#dev_name,#disk_list[disc_num].Item); else
		for (j1=0; j1<dev_disc_num; j1++;)
		{
			copystr(#dev_name, #sys_discs);                              // /rd/
			copystr(j1*304+ buf+72, #sys_discs+strlen(#sys_discs));      // /rd/1
			copystr("/", #sys_discs+strlen(#sys_discs));                 // /rd/1/
			copystr(#sys_discs,#disk_list[disc_num].Item);
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
	
	DrawBar(2,41,190,15,lpanel_col);		      //синий прямоугольник - сверху
	DrawBar(2,56,15,onTop(21,41),lpanel_col);	  //синий прямоугольник - слева       
	DrawBar(177,56,15,onTop(21,41),lpanel_col); //синий прямоугольник - справа
	//список дисков
	if (show_dev_name) Tip(56, "Devices", 78, "=");
		else Tip(56, "Devices", 78, "-");
	for (i=0;i<disc_num;i++)
	{
		DrawBar(17,i*16+74,160,17,0xFFFFFF); //белое
		DefineButton(17,i*16+74,159,16,100+i+BT_HIDE,0xFFFFFF); //создаём кнопки, а потом выводим названия дисков
		copystr("Unknown ",#disc_name); //изначально неизвесный носитель
		dev_icon=3; //по-умолчанию устройство выглядит как жестяк
		copystr(#disk_list[i].Item, #dev_name);
		dev_name[strlen(#dev_name)-1]='';
		IF (dev_name[1]=='r')  { dev_icon=0; copystr("RAM disk ",#disc_name); }
		IF (dev_name[1]=='c')  { dev_icon=1; copystr("CD-ROM ",#disc_name); }
		IF (dev_name[1]=='f')  { dev_icon=2; copystr("Floppy disk ",#disc_name); }
		IF (dev_name[1]=='t')  { dev_icon=4; copystr("Virtual disk ",#disc_name); }
		IF (dev_name[1]=='h') || (dev_name[1]=='b') copystr("Hard disk ",#disc_name); 
		copystr(#dev_name,#disc_name+strlen(#disc_name));
		//
		if (show_dev_name) WriteText(45,i*16+79,0x80,0,#disc_name,0);
			else WriteText(45,i*16+79,0x80,0,#dev_name,0);
		PutImage(dev_icon*14*13*3+#devices,14,13,21,i*16+76);
	}
}
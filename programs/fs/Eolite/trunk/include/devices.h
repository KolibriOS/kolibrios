//03.04.2012

path_string disk_list[20];
int disc_num;
dword devbuf;

void GetSystemDiscs()
{
	unsigned char dev_name[10], sys_discs[10];
	unsigned int i1, j1, dev_num, dev_disc_num;
	
	disc_num=0;
	if (devbuf) free(devbuf);
	devbuf= malloc(3112); //буфер где-то на 10 девайсов в левой панели
	ReadDir(19, devbuf, "/");
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
				strcpy(#disc_name, "RAM disk ");
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
			case 't':
				dev_icon=4;
				strcpy(#disc_name, "RAM disk ");
				
				//temp[0]=dev_name[4]; //ахуеть система
				//temp[1]=NULL;
				//DefineButton(17+143,i*16+74,16,16,StrToInt(#temp)+130+BT_HIDE+BT_NOFRAME,0xFFFFFF);
				//WriteText(45+121,i*16+79,0x80,0xAC0000,"-",0);
				//WriteText(45+121,i*16+79+1,0x80,0xAC0000,"-",0);
				break;
			default:
				dev_icon=3; //по-умолчанию устройство выглядит как жестяк но это неправильно
				strcpy(#disc_name, "Unknown ");				
		}
		strcat(#disc_name, #dev_name);
		if (show_dev_name) WriteText(45,i*16+79,0x80,0,#disc_name,0);
			else WriteText(45,i*16+79,0x80,0,#dev_name,0);
		PutImage(dev_icon*14*13*3+#devices,14,13,21,i*16+76);
	}
}
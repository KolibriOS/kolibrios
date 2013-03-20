//Leency 2008-2013


dword onLeft(dword right,left) {EAX=Form.width-right-left;}
dword onTop(dword down,up) {EAX=Form.height-GetSkinHeight()-down-up;}


void ShowMessage(dword message)
{
	DrawFlatButton(Form.width/2-13,160,200,80,0,0xFFB6B5, message);
	pause(150);
	List_ReDraw();
}


dword ConvertSize(dword bytes)
{
	unsigned char size_prefix[8], size_nm[4];
	if (bytes>=1073741824) strcpy(#size_nm, " Gb");
	else if (bytes>=1048576) strcpy(#size_nm, " Mb");
	else if (bytes>=1024) strcpy(#size_nm, " Kb");
	else strcpy(#size_nm, " b ");
	while (bytes>1023) bytes/=1024;
	strcpy(#size_prefix, itoa(bytes));
	strcat(#size_prefix, #size_nm);
	return #size_prefix;
}


dword col_palette[14] = {0xD2D3D3,0xD4D4D4,0xD6D5D6,0xD8D7D8,0xDAD8D9,0xDCDADB,
0xDFDCDD,0xE1DDDE,0xE2DEE0,0xE4DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1,0xE3DFE1};

inline fastcall void TVScroll() { //Прокрутка
	dword on_y, i;
	if (count<=0)
	{
		on_y = 57;
		scroll_size = onTop(22,58);
	}
	else
	{
		on_y = za_kadrom * onTop(22,57) / count +57;
		scroll_size=onTop(22,57) * f_visible - f_visible / count;
		if (scroll_size<20) scroll_size = 20; //устанавливаем минимальный размер скролла
		if (scroll_size>onTop(22,57)-on_y+56) || (za_kadrom+f_visible>=count) on_y=onTop(23+scroll_size,0); //для большого списка 
	}
	DrawFlatButton(onLeft(27,0),on_y,16,scroll_size,0,-1,"");//ползунок
	if (!scroll_used) for (i=0; i<13; i++) DrawBar(onLeft(25-i,0), on_y+2, 1, scroll_size-3, col_palette[13-i]);
	if (scroll_used)  for (i=0; i<13; i++) DrawBar(onLeft(25-i,0), on_y+2, 1, scroll_size-3, col_palette[i]);
	//поле до ползунка
	if (on_y>58) DrawBar(onLeft(26,0),57,15,1,      0xC7C9C9);
	DrawBar(onLeft(26,0),58,1, on_y-58,0xC7C9C9);
	DrawBar(onLeft(25,0),58,14,on_y-58,0xCED0D0);
	//поле после ползунка
	if (onTop(22,57)-scroll_size+55>on_y) DrawBar(onLeft(26,0),on_y+scroll_size+1,15,1,0xC7C9C9);
	DrawBar(onLeft(26,0),on_y+scroll_size+2,1,onTop(22,57)-scroll_size-on_y+55,0xC7C9C9);
	DrawBar(onLeft(25,0),on_y+scroll_size+2,14,onTop(22,57)-scroll_size-on_y+55,0xCED0D0);
}

void DrawFlatButton(dword x,y,width,height,id,color,text)
{
	int fill_h;
	DrawRectangle(x,y,width,height,col_border);
	DrawRectangle3D(x+1,y+1,width-2,height-2,0xFEFEFE,col_padding);
	PutPixel(x+width-1, y+1, col_work);
	if (color!=-1) DrawFilledBar(x+2, y+2, width-3, height-3);
	IF (id<>0)	DefineButton(x+1,y+1,width-2,height-2,id+BT_HIDE,0xEFEBEF);
	WriteText(-strlen(text)*6+width/2+x+1,height/2-3+y,0x80,0,text);
}

void DrawFilledBar(dword x, y, w, h)
{
	int i, fill_h;
	if (h <= 14) fill_h = h; else fill_h = 14;
	for (i=0; i<fill_h; i++) DrawBar(x, y+i, w, 1, col_palette[14-i]);	
	DrawBar(x, y+i, w, h-fill_h, col_palette[14-i]);		
}
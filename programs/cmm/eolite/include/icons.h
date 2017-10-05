_ini icons = { "/sys/File managers/icons.ini", "icons16" };

void DrawIconByExtension(dword file_path, extension, xx, yy, fairing_color)
{
	char BYTE_HEAD_FILE[4];
	char ext[512];
	int i;
	dword icon_n=0;

	if (extension)
	{
		strcpy(#ext, extension);
		strlwr(#ext);
		icon_n = icons.GetInt(#ext, 2);
	} 
	else if (file_path)
	{
			ReadFile(0,4,#BYTE_HEAD_FILE,file_path);
			IF(DSDWORD[#BYTE_HEAD_FILE]=='KCPK')||(DSDWORD[#BYTE_HEAD_FILE]=='UNEM') icon_n = icons.GetInt("kex", 2);
	}
	if (fairing_color==col_selec)
	{
		img_draw stdcall(icons16_selected.image, xx, yy, 16, 16, 0, icon_n*16);
		IconFairing(icon_n, xx, yy, fairing_color);
	}
	else 
	{
		img_draw stdcall(icons16_default.image, xx, yy, 16, 16, 0, icon_n*16);
	}
}


void IconFairing(dword filenum, x,y, color)
{
	switch(filenum)
	{
		case 0: //folder
		case 22: //<up>
			DrawBar(x+7,y,8,2,color);
			IF (filenum==22) PutPixel(x+10,y+1,0x1A7B17); //green arrow part
			DrawBar(x,y+13,15,2,color);
			PutPixel(x,y,color);
			PutPixel(x+6,y,color);
			PutPixel(x+14,y+2,color);
			PutPixel(x,y+12,color);
			PutPixel(x+14,y+12,color);
			return;
		case 13: //html
			DrawBar(x,y,1,7,color);
			DrawBar(x+1,y,1,6,color);
			DrawBar(x,y+10,1,5,color);
			DrawBar(x+1,y+11,1,4,color);
			return;
		case 12: //font
			DrawBar(x+1,y+1,1,13,color);
			DrawBar(x+2,y+1,1,11,color);
			DrawBar(x+3,y+1,1,10,color);
			DrawBar(x+4,y+1,1,9,color);
			DrawBar(x+5,y+1,1,7,color);
			DrawBar(x+6,y+1,1,5,color);
			DrawBar(x+7,y+1,1,4,color);
			DrawBar(x+8,y+1,1,2,color);
			DrawBar(x+14,y+1,1,13,color);
			DrawBar(x+13,y+1,1,11,color);
			PutPixel(x+9,y+6,color);
			DrawBar(x+8,y+10,2,1,color);
			DrawBar(x+7,y+11,2,3,color);
			return;
		case 23: //nes
			DrawBar(x,y+11,1,2,color);
			DrawBar(x+15,y+11,1,2,color);
			DrawBar(x,y+13,16,1,color);
			return;
	}
}

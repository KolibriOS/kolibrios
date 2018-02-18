_ini icons = { "/sys/File managers/icons.ini", "icons16" };

void DrawIconByExtension(dword file_path, extension, xx, yy, fairing_color)
{
	char BYTE_HEAD_FILE[4];
	char ext[512];
	int i;
	dword icon_n=2; // set default icon

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
	//0 = folder
	//22 = forder with up arrow
	if (filenum == 0) || (filenum == 22)
	{
		DrawBar(x+7,y+1,8,2,color);
		DrawBar(x,y+14,15,2,color);
		PutPixel(x,y+1,color);
		PutPixel(x+6,y+1,color);
		PutPixel(x+14,y+3,color);
		PutPixel(x,y+13,color);
		PutPixel(x+14,y+13,color);
	}
	if (filenum == 22) PutPixel(x+10,y+2,0x1A7B17); //green arrow part
}

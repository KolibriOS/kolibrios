_ini icons_ini = { "/sys/File managers/icons.ini", NULL };

void DrawIconByExtension(dword file_path, extension, xx, yy, fairing_color)
{
	char BYTE_HEAD_FILE[4];
	char ext[512];
	int icon_n = 2;
	dword selected_image;
	dword default_image;
	dword default_icon;

	if (big_icons.checked) {
		icons_ini.section = "icons32";
		selected_image = icons32_selected.image;
		default_image = icons32_default.image;
		default_icon=95;
	} else {
		icons_ini.section = "icons16";
		selected_image = icons16_selected.image;
		default_image = icons16_default.image;
		default_icon=2;
	}

	if (chrnum(file_path, '/')==2) {
		if (ESBYTE[file_path+1]=='/') ext[0] = ESBYTE[file_path+2];
			else ext[0] = ESBYTE[file_path+1];
		ext[1] = '\0';
		if (big_icons.checked) {
			icons_ini.section = "drives32";
			icon_n = icons_ini.GetInt(#ext, 50);
		} else {
			icons_ini.section = "drives16";
			icon_n = icons_ini.GetInt(#ext, 50);
		}
	} else if (extension) {
		strcpy(#ext, extension);
		strlwr(#ext);
		icon_n = icons_ini.GetInt(#ext, default_icon);
	} else if (file_path) {
		ReadFile(0,4,#BYTE_HEAD_FILE,file_path);
		IF(DSDWORD[#BYTE_HEAD_FILE]=='KCPK')||(DSDWORD[#BYTE_HEAD_FILE]=='UNEM') 
			icon_n = icons_ini.GetInt("kex", 2);
	}

	if (fairing_color==col.selec) {
		img_draw stdcall(selected_image, xx, yy, icon_size, icon_size, 0, icon_n*icon_size);
	} else {
		img_draw stdcall(default_image, xx, yy, icon_size, icon_size, 0, icon_n*icon_size);
	}
}


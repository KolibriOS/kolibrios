_ini icons_ini = { "/sys/File managers/icons.ini", NULL };

struct ICONS_INI {
	collection exts;
	collection_int i18;
	collection_int i32;
	int get();
} ini_icons;

int ICONS_INI::get(dword _file_path, _ext, int size)
{
	char file_4bytes[4], ext[7];
	int  ext_pos;
	if (!_ext) {
		ReadFile(0,4,#file_4bytes,_file_path);
		IF(DSDWORD[#file_4bytes]=='KCPK')
		||(DSDWORD[#file_4bytes]=='UNEM') {
			_ext = "kex";
		}
	}
	strlcpy(#ext, _ext, sizeof(ext));
	strlwr(#ext);
	ext_pos = exts.get_pos_by_name(#ext);
	if (ext_pos != -1) {
		if (size == 18) return i18.get(ext_pos);
		else if (size == 32) return i32.get(ext_pos);
	} else {
		exts.add(#ext);
			icons_ini.section = "icons16";
			i18.set(exts.count-1, icons_ini.GetInt(#ext, 2));

			icons_ini.section = "icons32";
			i32.set(exts.count-1, icons_ini.GetInt(#ext, 95));
		return get(#ext, size);
	}
}

void DrawIconByExtension(dword file_path, extension, xx, yy, fairing_color)
{
	char ext[7];
	int icon_n = 2;
	dword selected_image;
	dword default_image;

	if (ESBYTE[file_path+1]!='k') && (ESBYTE[file_path+1]!='s') && (chrnum(file_path, '/')==2) {
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
	} else {
		icon_n = ini_icons.get(file_path, extension, icon_size);
	}

	if (big_icons.checked) {
		selected_image = icons32_selected.image;
		default_image = icons32_default.image;
	} else {
		selected_image = icons16_selected.image;
		default_image = icons16_default.image;
	}

	if (fairing_color==col.selec) {
		img_draw stdcall(selected_image, xx, yy, icon_size, icon_size, 0, icon_n*icon_size);
	} else {
		img_draw stdcall(default_image, xx, yy, icon_size, icon_size, 0, icon_n*icon_size);
	}
}


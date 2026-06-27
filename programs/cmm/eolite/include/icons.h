
struct ICONS_INI {
	collection exts;
	collection_int i18;
	collection_int i32;
	byte loaded;
	int get();
	void load();
	void put();
} ini_icons;

// Stores the icon number for a single extension, keeping the exts/i18/i32
// collections aligned by position regardless of the order keys are read in.
void ICONS_INI::put(dword _ext, int size, icon_n)
{
	int ext_pos = exts.get_pos_by_name(_ext);
	if (ext_pos == -1) {
		exts.add(_ext);
		ext_pos = exts.count - 1;
	}
	if (size == 18) i18.set(ext_pos, icon_n);
	else i32.set(ext_pos, icon_n);
}

// Callback for ini_enum_keys: key_name is the icon number, key_value is a
// comma-separated list of extensions sharing that icon, e.g.
//   3=doc,docx,exc,inf,log,ob07,odt,rtf,txt,wtx
byte icons_ini_enum_key(dword key_value, key_name, sec_name, f_name)
{
	char ext[7];
	int  icon_n, size, i;
	dword src;

	icon_n = atoi(key_name);
	if (streq(sec_name, "icons18")) size = 18; else size = 32;

	src = key_value;
	while (ESBYTE[src]) {
		i = 0;
		while (ESBYTE[src]) && (ESBYTE[src] != ',') {
			if (ESBYTE[src] != ' ') && (i < 6) {
				ext[i] = ESBYTE[src];
				i++;
			}
			src++;
		}
		ext[i] = '\0';
		if (i) {
			strlwr(#ext);
			ini_icons.put(#ext, size, icon_n);
		}
		if (ESBYTE[src] == ',') src++;
	}
	return true;
}

// Reads the whole icon map once into the collections.
void ICONS_INI::load()
{
	if (loaded) return;
	loaded = true;
	ini_enum_keys stdcall (icons_ini.path, "icons18", #icons_ini_enum_key);
	ini_enum_keys stdcall (icons_ini.path, "icons32", #icons_ini_enum_key);
}

int ICONS_INI::get(dword _file_path, _ext, int size)
{
	char file_4bytes[4], ext[7];
	int  ext_pos;
	load();
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
		if (size == 32) return i32.get(ext_pos);
	}
	if (size == 18) return 2;
	return 95;
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
			icons_ini.section = "drives18";
			icon_n = icons_ini.GetInt(#ext, 50);
		}
	} else {
		icon_n = ini_icons.get(file_path, extension, icon_size);
	}

	if (big_icons.checked) {
		selected_image = icons32_selected.image;
		default_image = icons32_default.image;
	} else {
		selected_image = icons18_selected.image;
		default_image = icons18_default.image;
	}

	if (fairing_color==col.selec) {
		img_draw stdcall(selected_image, xx, yy, icon_size, icon_size, 0, icon_n*icon_size);
	} else {
		img_draw stdcall(default_image, xx, yy, icon_size, icon_size, 0, icon_n*icon_size);
	}
}


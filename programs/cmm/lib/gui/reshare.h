
:bool draw_icon_32(dword _x,_y, _bg, _icon_n) {
	static dword bg;
	static dword shared;
	static dword icon32mem;
	dword size;
	if (!shared) || (bg!=_bg) {
		if (shared = memopen("ICONS32", NULL, SHM_READ)) 
		{
			size = EDX;
			if (!icon32mem) icon32mem = malloc(size);
			memmov(icon32mem, shared, size);
			EDX = icon32mem + size;
			EAX = bg = _bg;
			for (ESI = icon32mem; ESI < EDX; ESI += 4) {
				if (DSDWORD[ESI]==0x00000000) DSDWORD[ESI] = EAX;
			}			
		}
	}
	if (icon32mem) {
		PutPaletteImage(32*32*4*_icon_n + icon32mem, 32, 32,_x, _y, 32, 0);
		return true;
	}
	return false;
}

:bool draw_icon_16(dword _x,_y, _icon_n) {
	static dword shared_i16;
	if (!shared_i16) shared_i16 = memopen("ICONS18", NULL, SHM_READ);
	if (shared_i16) {
		PutPaletteImage(18*18*4*_icon_n + shared_i16, 18, 18,_x, _y, 32, 0);
		return true;
	}
	return false;
}

:bool draw_icon_16w(dword _x,_y, _icon_n) {
	static dword shared_i16w;
	if (!shared_i16w) shared_i16w = memopen("ICONS18W", NULL, SHM_READ);
	if (shared_i16w) {
		PutPaletteImage(18*18*4*_icon_n + shared_i16w, 18, 18,_x, _y, 32, 0);
		return true;
	}
	return false;
}

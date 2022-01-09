//<= imgsrc, imgsize, color_old_1, color_new_1, color_old_2, color_new_2
inline fastcall replace_2cols(EDI, EDX, ESI, ECX, EBX, EAX) 
{
    EDX += EDI; //imgsrc + imgsize;
    WHILE (EDI < EDX) {
        IF (DSDWORD[EDI]==ESI) DSDWORD[EDI] = ECX;
        ELSE IF (DSDWORD[EDI]==EBX) DSDWORD[EDI] = EAX;
        EDI += 4;
    }
}

:unsigned int DrawTopPanelButton(dword _button_id, _x, _y, signed int _icon_n, bool pressed)
{
	#define TSZE 25
	static dword lightest, i16_mem, old_work_light;
	dword i16_size;
	if (!lightest) || (old_work_light != sc.light) {
		old_work_light = sc.light;
		lightest = MixColors(sc.light, 0xFFFfff, skin_is_dark()*155 + 20);
		if (ESI = memopen("ICONS18", NULL, SHM_READ)) {
			i16_size = EDX;
			i16_mem = malloc(i16_size);
			memmov(i16_mem, ESI, i16_size);
			replace_2cols(i16_mem, i16_size, 0xffFFFfff, sc.light, 0xffCACBD6, sc.dark);			
		}
	}
	DrawWideRectangle(_x+1, _y+1, TSZE, TSZE, 5, sc.light);
	DefineHiddenButton(_x, _y, TSZE+1, TSZE+1, _button_id);
	if (_icon_n==-1) {
		DrawBar(_x+6, _y+5, 16, 16, sc.light);
		DrawBar(_x+6, _y+7, 15, 3, sc.line);
		$add ecx,5*65536
		$int 64
		$add ecx,5*65536
		$int 64
	} else {
		if (i16_mem) PutPaletteImage(18*18*4*_icon_n + i16_mem, 
			18, 18, TSZE/2-9+2+_x, TSZE/2-9+1+_y+pressed, 32, 0);
	}

	if (!pressed) {
		DrawOvalBorder(_x, _y, TSZE, TSZE, lightest, sc.line, sc.light, sc.work);
	} else {
		DrawOvalBorder(_x, _y, TSZE, TSZE, sc.line, sc.light, sc.dark, sc.work);
		PutShadow(_x+1, _y+1, TSZE, TSZE, true, 2);
	}

	return _x;
}
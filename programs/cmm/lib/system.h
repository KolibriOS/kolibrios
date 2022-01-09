#ifndef INCLUDE_SYSTEM_H
#define INCLUDE_SYSTEM_H

:struct COLORS {
dword
	nonset1,
	taskbar_color,
	dark,
	light,
	window_title,
	work,
	button,
	button_text,
	work_text,
	line;
	void get();
} sc;

:void COLORS::get()
{
	EAX = 48;
	EBX = 3;
	ECX = #nonset1;
	EDX = 40;
	$int 0x40
}

#endif
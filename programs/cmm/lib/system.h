#ifndef INCLUDE_SYSTEM_H
#define INCLUDE_SYSTEM_H

:struct COLORS {
dword
	nonset1,
	taskbar_color,
	work_dark,
	work_light,
	window_title,
	work,
	work_button,
	work_button_text,
	work_text,
	work_graph;
	void get();
};

:void COLORS::get()
{
	EAX = 48;
	EBX = 3;
	ECX = #nonset1;
	EDX = 40;
	$int 0x40
}

:struct SYSTEM
{
	COLORS color;
} system;

#endif
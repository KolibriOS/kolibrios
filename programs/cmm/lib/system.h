#ifndef INCLUDE_SYSTEM_H
#define INCLUDE_SYSTEM_H

:struct COLORS
{
	dword frame,grab,grab_button,grab_button_text,grab_text,
	      work,work_button,work_button_text,work_text,work_graph;
	void get();
};

:void COLORS::get()
{
	EAX = 48;
	EBX = 3;
	ECX = #frame;
	EDX = 40;
	$int 0x40
}

:struct SYSTEM
{
	COLORS color;
} system;

#endif
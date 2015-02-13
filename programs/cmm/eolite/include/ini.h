//INI parser in C--, GPL licence.
//Leency - 2012

void GetIni(byte onload)
{
	dword eolite_ini_path = abspath("Eolite.ini"); 
	ini_get_color stdcall (eolite_ini_path, "Config", "SelectionColor", 0x94AECE);
	edit2.shift_color = EAX;
	col_selec = EAX;
	ini_get_int stdcall (eolite_ini_path, "Config", "LineHeight", 18);
	files.line_h = EAX;
	ini_get_int stdcall (eolite_ini_path, "Config", "ShowDeviceName", 1);
	show_dev_name = EAX;
	ini_get_int stdcall (eolite_ini_path, "Config", "RealFileNamesCase", 0);
	real_files_names_case = EAX;
}


void Write_Error(int error_number)
{
	char error_message[500];
	dword ii;
	if (files.current>=0) Line_ReDraw(0xFF0000, files.current);
	pause(5);
	strcpy(#error_message, "\"Eolite\n");
	ii = get_error(error_number);
	strcat(#error_message, ii);
	strcat(#error_message, "\" -tE");
	notify(#error_message);
}

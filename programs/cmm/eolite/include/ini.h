//INI parser in C--, GPL licence.
//Leency - 2012

#define COMMENT	0
#define SECTION	1
#define PARAM	2
#define OPTION	3


void GetIni(byte onload)
{
	byte section[32], parametr[32], option[256], InfType=0;
	char bukva[2];
	int errornum, tj;
	static dword buff, fsize;
	if (onload==1)
	{
		free(buff);
		if (!GetFile(#buff, #fsize, abspath("Eolite.ini"))) notify("Eolite.ini not found. Defaults will be used.");
	}
	
	ini_get_color stdcall (abspath("Eolite.ini"), "Config", "SelectionColor", 0x94AECE);
	edit2.shift_color = EAX;
	ini_get_int stdcall (abspath("Eolite.ini"), "Config", "LineHeight", 18);
	files.line_h = EAX;
	ini_get_int stdcall (abspath("Eolite.ini"), "Config", "ShowDeviceName", 1);
	show_dev_name = EAX;
	ini_get_int stdcall (abspath("Eolite.ini"), "Config", "RealFileNamesCase", 0);
	real_files_names_case = EAX;
	ini_get_int stdcall (abspath("Eolite.ini"), "Config", "DrwRamDiskSpace", 0);
	drw_ram_disk_space = EAX;
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


dword StrToCol(char* htmlcolor)
{
  dword j, color=0;
  char ch=0x00;
  
  FOR (j=0; j<6; j++)
  {
    ch=ESBYTE[htmlcolor+j];
    IF ((ch>='0') && (ch<='9')) ch -= '0';
    IF ((ch>='A') && (ch<='F')) ch -= 'A'-10;
    IF ((ch>='a') && (ch<='f')) ch -= 'a'-10;
    color = color*0x10 + ch;
  }
   return color;
}
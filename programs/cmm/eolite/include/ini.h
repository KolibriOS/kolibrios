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
	for (tj=0; tj<fsize; tj++;) 
	{   
		bukva = ESBYTE[buff+tj];
		switch (bukva)
		{
			case ';': InfType=COMMENT; break;				
			case '[': InfType=SECTION; section=NULL; break;
			case ']': InfType=PARAM; break;
			case '=': InfType=OPTION; break;
			case 0x0a:
			case 0x0d:
				InfType=PARAM;
				if (!strcmp(#parametr,"SelectionColor")) edit2.shift_color=col_selec=StrToCol(#option);
				if (!strcmp(#parametr,"LineHeight")) files.line_h = atoi(#option);
				if (!strcmp(#parametr,"ShowDeviceName")) show_dev_name=atoi(#option);
				if (!strcmp(#parametr,"RealFileNamesCase")) real_files_names_case=atoi(#option);
				if (!strcmp(#parametr,"DrwRamDiskSpace")) drw_ram_disk_space=atoi(#option);
				
				if (parametr) && (!strcmpi(#file_name+strrchr(#file_name,'.'),#parametr)) && (!onload)
				{
					errornum = RunProgram(#option,#file_path);
					if (errornum<0)
					{
						if (errornum==-5) ShowOpenWithDialog(); else Write_Error(errornum);
					}
					return;
				}
				parametr=option=NULL;
				break;
			default:
				IF (InfType==SECTION) chrcat(#section, bukva);
				IF (InfType==PARAM) chrcat(#parametr, bukva);
				IF (InfType==OPTION) chrcat(#option, bukva);
		}
	}
	if (file_path) && (!onload)
	{
		errornum = RunProgram(#file_path,NULL); 
		if (errornum==-31) menu_action(201); else if (errornum<0) Write_Error(errornum);
		return;
	}
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
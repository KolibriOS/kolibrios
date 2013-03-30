//INI parser in C--, GPL licence.
//Leency - 2012

#define COMMENT	0
#define SECTION	1
#define PARAM	2
#define OPTION	3

unsigned char *ERROR_TEXT[]={
"Code #0 - No error",
"Error #1 - Base or partition of a hard disk is not defined",
"Error #2 - Function isn't supported for this file system",
"Error #3 - Unknown file system",
"Error #4 - Reserved, is never returned",
"Error #5 - File or folder not found",
"Error #6 - End of file, EOF",
"Error #7 - Pointer lies outside of application memory",
"Error #8 - Too less disk space",
"Error #9 - FAT table is destroyed",
"Error #10 - Access denied",
"Error #11 - Device error",
0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 
"Error #30 - Not enough memory",
"Error #31 - File is not executable",
"Error #32 - Too many processes",
0}; 

void GetIni(byte onload)
{
	byte section[32], parametr[32], option[256], InfType=0;
	char bukva[2];
	int errornum, tj;
	static dword buff, fsize;
	//читаем файл
	if (onload==1)
	{
		free(buff);
		if (!GetFile(#buff, #fsize, abspath("Eolite.ini"))) notify("Eolite.ini not found. Defaults will be used.");
	}
	//парсим его
	for (tj=0; tj<fsize; tj++;) 
	{   
		bukva = ESBYTE[buff+tj];
		switch (bukva)
		{
			case ';':
				InfType=COMMENT;
				break;				
			case '[':
				InfType=SECTION;
				section=NULL;
				break;
			case ']':
				InfType=PARAM;
				break;
			case '=':
				InfType=OPTION;
				break;
			case 0x0a:
			case 0x0d:
				InfType=PARAM;
				IF (!strcmp(#parametr,"SelectionColor")) edit2.shift_color=col_selec=StrToCol(#option);
				IF (!strcmp(#parametr,"LineHeight")) BUTTON_HEIGHT=atoi(#option);
				IF (!strcmp(#parametr,"ShowDeviceName")) show_dev_name=atoi(#option);
				
				/*if (!strcmp(#section,"UserDirectories")) && (parametr) && (onload)
				{
					copystr(#parametr, #disk_list[disc_num].Item);
					disc_num++;
				}*/
				
				IF (parametr) && (!strcmp(#file_name+strrchr(#file_name,'.'),#parametr)) && (!onload)
				{
					errornum=RunProgram(#option,#file_path);
					IF (errornum<0) Write_Error(errornum); //если ошибочка вышла при запуске
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
		errornum=RunProgram(#file_path,NULL); 
		if (errornum<0) Write_Error(errornum); //если ошибочка вышла при запуске
	}
}


void Write_Error(int error_number)
{
	char error[256];

	if (error_number<0) error_number=-1*error_number;
	
	if (error_number<33)
		strcpy(#error, ERROR_TEXT[error_number]);
	else
		{
			strcpy(#error, itoa(error_number));
			strcat(#error, " - Unknown error number O_o");
		}
	if (curbtn>=0) Line_ReDraw(0xFF0000, curbtn);
	pause(5);
	notify(#error);
}


dword StrToCol(char* htmlcolor)
{
	dword color;
	char j, ch;

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
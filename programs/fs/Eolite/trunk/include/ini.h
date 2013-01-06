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
"Error #8 - FAT table is destroyed",
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
	byte section[32]='', parametr[32]='', option[256]='', InfType=0;
	char bukva[2];
	int errornum;
	dword fsize, tj;
	static dword buff;
	//читаем файл
	IF (onload==1)
	{
		free(buff);
		buff = malloc(12000);
		copystr(".ini", #program_path+strlen(#program_path));
	}

	ReadFile(0, 12000, buff, #program_path);
	IF (EAX<>6) ReadFile(0, 12000, buff, "/sys/File managers/Eolite.ini");
	IF (EAX<>6) //если файла с настройками тупо нет печалька
	{
		IF (onload==1) notify("Eolite.ini not found. Defaults will be used.");
		IF (onload==0) goto RUN_AS_PROGRAM;
	}
	fsize=EBX;
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
				section='';
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
				IF (!strcmp(#parametr,"SelectionColor")) col_selec=StrToCol(#option);
				IF (!strcmp(#parametr,"LineHeight")) BUTTON_HEIGHT=StrToInt(#option);
				IF (!strcmp(#parametr,"ShowDeviceName")) show_dev_name=StrToInt(#option);
				
				/*if (!strcmp(#section,"UserDirectories")) && (parametr) && (onload)
				{
					copystr(#parametr, #disk_list[disc_num].Item);
					disc_num++;
				}*/
				
				IF (parametr) && (!strcmp(#file_name+strchr(#file_name,'.'),#parametr)) {
					errornum=RunProgram(#option,#file_path);
					IF (errornum<0) //если ошибочка вышла при запуске
					{
						//WriteDebug(#option);
						Write_Error(errornum); 
					}
					return;
				}
				parametr=option='';
				break;
			default:
				IF (InfType==SECTION) copystr(#bukva,#section+strlen(#section));
				IF (InfType==PARAM) copystr(#bukva,#parametr+strlen(#parametr));
				IF (InfType==OPTION) copystr(#bukva,#option+strlen(#option));
		}
	}
	RUN_AS_PROGRAM: 
	IF (file_path) errornum=RunProgram(#file_path,''); 
	IF (errornum<0) //если ошибочка вышла при запуске
	{
		Write_Error(errornum); 
	}
}


void Write_Error(int error_number)
{
	char error[256];

	if (error_number<0) error_number=-1*error_number;
	
	if (error_number<33)
		copystr(ERROR_TEXT[error_number], #error);
	else
		{
			copystr(IntToStr(error_number), #error);
			copystr(" - Unknown error number O_o", #error+strlen(#error));
		}
	if (curbtn>=0) Line_ReDraw(0xFF0000, curbtn);
	Pause(5);
	notify(#error);
	//DrawBar(192,onTop(0, BUTTON_HEIGHT+7),onLeft(27,192),BUTTON_HEIGHT,0xFF0000);
	//WriteText(205,onTop(-5, BUTTON_HEIGHT+7),0x80,0xFFFFFF,#error,0);
}
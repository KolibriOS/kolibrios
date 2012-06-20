//INI parser in C--, GPL licence.
//Leency - 2012

#define COMMENT	0
#define SECTION	1
#define PARAM	2
#define OPTION	3

char NOTIFY_PATH[7]="@notify";

void GetIni(byte onload)
{
	byte section[32]='', parametr[32]='', option[256]='', InfType=0;
	word bukva[1];
	int errornum;
	dword buff, fsize, tj;
	//читаем файл
	buff = malloc(1048576);
	IF (onload==1) copystr(".ini", #program_path+strlen(#program_path));
	ReadFile(0, 1048576, buff, #program_path);
	IF (EAX<>6) //если файла с настройками нет в папке с программой смотрим в папке по-умолчанию
		ReadFile(0, 1048576, buff, "/sys/File managers/Eolite.ini");
	IF (EAX<>6) //если файла с настройками тупо нет печалька
	{
		IF (onload==1) RunProgram(#NOTIFY_PATH, "Eolite.ini not found. Defaults will be used.");
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
				IF (!strcmp(#parametr,"SelectionColor")) videlenie=StrToCol(#option);
				IF (!strcmp(#parametr,"LineHeight")) BUTTON_HEIGHT=StrToInt(#option);
				IF (!strcmp(#parametr,"ShowDeviceName")) show_dev_name=StrToInt(#option);
				
				/*if (!strcmp(#section,"UserDirectories")) && (parametr) && (onload)
				{
					copystr(#parametr, #disk_list[disc_num].Item);
					disc_num++;
				}*/
				
				IF (parametr) && (!strcmp(#file_name+find_symbol(#file_name,'.'),#parametr)) {
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


///////////////////////////////////////////
/// нужно чтобы просто возвращало текст ///
///////////////////////////////////////////

void Write_Error(int error_number)
{
	char error[256];
	if (error_number<0) error_number=-1*error_number;
	switch (error_number)
	{
		case 2:	copystr("Error #2 - Function isn't supported for this file system", #error);
				break;				
		case 3:	copystr("Error #3 - Unknown file system", #error);
				break;
		case 5: copystr("Error #5 - File or folder not found", #error);
				break;
		case 6:	copystr("Error #6 - End of file, EOF", #error);
				break;
		case 7:	copystr("Error #7 - Pointer lies outside of application memory", #error);
				break;		
		case 8:	copystr("Error #8 - FAT table is destroyed", #error);
				break;		
		case 9: copystr("Error #9 - FAT table is destroyed", #error);
				break;
		case 10:copystr("Error #10 - Access denied", #error);
				break;				
		case 11:copystr("Error #11 - Device error", #error);
				break;
		case 30:copystr("Error #30 - Not enough memory", #error);
				break;
		case 31:copystr("Error #31 - File is not executable", #error);
				break;
		case 32:copystr("Error #32 - Too many processes", #error);
				break;
		default:copystr(IntToStr(error_number), #error);
				copystr(" - Unknown error number O_o", #error+strlen(#error));
	}
	if (curbtn>=0) Line_ReDraw(0xFF0000, curbtn);
	Pause(5);
	RunProgram(#NOTIFY_PATH, #error);
	//DrawBar(192,onTop(0, BUTTON_HEIGHT+7),onLeft(27,192),BUTTON_HEIGHT,0xFF0000);
	//WriteText(205,onTop(-5, BUTTON_HEIGHT+7),0x80,0xFFFFFF,#error,0);

	
}

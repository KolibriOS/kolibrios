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
	IF (onload==1) copystr(".ini", #program_path+strlen(#program_path)); //facepalm
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
						Write_Debug_Error(errornum); 
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
		//WriteDebug(#file_path);
		Write_Debug_Error(errornum); 
	}
}


///////////////////////////////////////////
/// нужно чтобы просто возвращало текст ///
///////////////////////////////////////////

void Write_Debug_Error(int error_number)
{
	char unknown_error[8];
	
	if (error_number<0) error_number=-1*error_number;
	switch (error_number) //извесная ошибка - пишем какая и уходим
	{
		CASE 2:	RunProgram(#NOTIFY_PATH, "Eolite: error 2 - Function is not supported for the given file system");
				return;				
		CASE 3:	RunProgram(#NOTIFY_PATH, "Eolite: error 3 - Unknown file system");
				return;
		CASE 5: RunProgram(#NOTIFY_PATH, "Eolite: error 5 - File or folder not found");
				return;
		CASE 6:	RunProgram(#NOTIFY_PATH, "Eolite: error 6 - End of file, EOF");
				return;
		CASE 7:	RunProgram(#NOTIFY_PATH, "Eolite: error 7 - Pointer lies outside of application memory");
				return;		
		case 8:	RunProgram(#NOTIFY_PATH, "Eolite: error 8 - FAT table is destroyed");
				return;		
		case 9: RunProgram(#NOTIFY_PATH, "Eolite: error 9 - FAT table is destroyed");
				return;
		case 10:RunProgram(#NOTIFY_PATH, "Eolite: error 10 - Access denied");
				RETURN;				
		case 11:RunProgram(#NOTIFY_PATH, "Eolite: error 11 - Device error");
				RETURN;
		case 30:RunProgram(#NOTIFY_PATH, "Eolite: error 30 - Not enough memory");
				RETURN;
		case 31:RunProgram(#NOTIFY_PATH, "Eolite: error 31 - File is not executable");
				RETURN;
		case 32:RunProgram(#NOTIFY_PATH, "Eolite: error 32 - Too many processes");
				RETURN;
		default:copystr(IntToStr(error_number), #unknown_error);
				copystr(" - Unknown error number O_o", #unknown_error+strlen(#unknown_error));
				RunProgram(#NOTIFY_PATH, #unknown_error);
	}
}

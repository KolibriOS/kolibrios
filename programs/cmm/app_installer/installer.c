#define MEMSIZE 731935 + 200200
#include "..\lib\strings.h"
#include "..\lib\file_system.h"

char app_name[] = "KingsBounty";

//file_listing.h must be generated using generate_file_listing.bat
#include "file_listing.h"

:dword makepath(dword basic_path, relative_path)
{
	char absolute_path[4096];
	strcpy(#absolute_path, basic_path);
	strcat(#absolute_path, relative_path);	
	return #absolute_path;
}

void main()
{
if (dir_exists("/kolibrios")==false) {
	die(
		"'/kolibrios/ folder is not mounted!\nPlease run APP+ on desktop.\nYou must use ISO distro.'E");
}

CreateDir("/tmp0/1/DOS");
CreateDir(sprintf(#param, "/tmp0/1/DOS/%s", #app_name));

if (EAX!=0) {
	die("'/tmp0/1/ is not mounted!\nPlease run TMPDISK to add it.'E");
}

WriteFile(sizeof(file0), #file0, makepath("/tmp0/1/DOS/", FILE_NAME_0));
WriteFile(sizeof(file1), #file1, makepath("/tmp0/1/DOS/", FILE_NAME_1));
#ifdef FILE_NAME_2
WriteFile(sizeof(file2), #file2, makepath("/tmp0/1/DOS/", FILE_NAME_2));
#endif
#ifdef FILE_NAME_3
WriteFile(sizeof(file3), #file3, makepath("/tmp0/1/DOS/", FILE_NAME_3));
#endif
#ifdef FILE_NAME_4
WriteFile(sizeof(file4), #file4, makepath("/tmp0/1/DOS/", FILE_NAME_4));
#endif
#ifdef FILE_NAME_5
WriteFile(sizeof(file5), #file5, makepath("/tmp0/1/DOS/", FILE_NAME_5));
#endif
#ifdef FILE_NAME_6
WriteFile(sizeof(file6), #file6, makepath("/tmp0/1/DOS/", FILE_NAME_6));
#endif
#ifdef FILE_NAME_7 
WriteFile(sizeof(file7), #file7, makepath("/tmp0/1/DOS/", FILE_NAME_7));
#endif
#ifdef FILE_NAME_8
WriteFile(sizeof(file8), #file8, makepath("/tmp0/1/DOS/", FILE_NAME_8));
#endif
#ifdef FILE_NAME_9
WriteFile(sizeof(file9), #file9, makepath("/tmp0/1/DOS/", FILE_NAME_9));
#endif

notify(sprintf(#param, "'%s\nInstalled to /tmp0/1/DOS/\nEnjoy the game!'tO", #app_name));
RunProgram("/sys/@open", sprintf(#param, "/tmp0/1/DOS/%s/PLAY.sh", #app_name));

ExitProcess();
}

stop:
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
if (dir_exists("/kolibrios")==false) 
die("'/kolibrios/ folder is not mounted!
Please run APP+ on desktop.
You must use ISO distro.'E");

CreateDir("/tmp0/1/DOS");
CreateDir(sprintf(#param, "/tmp0/1/DOS/%s", #app_name));

if (EAX!=0) {
	die("'/tmp0/1/ is not mounted!\nPlease run TMPDISK to add it.'E");
}

CreateFile(sizeof(file0), #file0, makepath("/tmp0/1/DOS/", FILE_NAME_0));

notify(sprintf(#param, "'%s\nInstalled to /tmp0/1/DOS/\nEnjoy the game!'tO", #app_name));
RunProgram("/sys/@open", sprintf(#param, "/tmp0/1/DOS/%s/PLAY.sh", #app_name));

ExitProcess();
}

stop:
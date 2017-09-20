#define MEMSIZE 731935 + 200200
#include "..\lib\strings.h"
#include "..\lib\file_system.h"

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
	CreateDir("/tmp0/1/DOS/KingsBounty");

	if (EAX!=0) {
		die("'/tmp0/1/ is not mounted!\nPlease run TMPDISK to add it.'E");
	}

	WriteFile(sizeof(file0), #file0, makepath("/tmp0/1/DOS/", FILE_NAME_0));
	WriteFile(sizeof(file1), #file1, makepath("/tmp0/1/DOS/", FILE_NAME_1));
	WriteFile(sizeof(file2), #file2, makepath("/tmp0/1/DOS/", FILE_NAME_2));
	WriteFile(sizeof(file3), #file3, makepath("/tmp0/1/DOS/", FILE_NAME_3));
	WriteFile(sizeof(file4), #file4, makepath("/tmp0/1/DOS/", FILE_NAME_4));
	WriteFile(sizeof(file5), #file5, makepath("/tmp0/1/DOS/", FILE_NAME_5));
	WriteFile(sizeof(file6), #file6, makepath("/tmp0/1/DOS/", FILE_NAME_6));
	WriteFile(sizeof(file7), #file7, makepath("/tmp0/1/DOS/", FILE_NAME_7));
	WriteFile(sizeof(file8), #file8, makepath("/tmp0/1/DOS/", FILE_NAME_8));
	WriteFile(sizeof(file9), #file9, makepath("/tmp0/1/DOS/", FILE_NAME_9));

	notify("'KingsBounty\nInstalled succesfull.\nEnjoy the game!'tO");
	RunProgram("/sys/@open", "/tmp0/1/DOS/KingsBounty/PLAY.sh");

	ExitProcess();
}

stop:
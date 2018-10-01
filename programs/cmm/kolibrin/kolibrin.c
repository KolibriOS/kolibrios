#define MEMSIZE 0x9000
#include "..\lib\strings.h" 
#include "..\lib\mem.h"
#include "..\lib\fs.h"


void main()
{   
	dword dirbuf, fcount, filename, i;
	dword dirbuf2, fcount2, filename2, j;
	char drive_name[4096];
	char install_path[4096];

	pause(200);
	GetDir(#dirbuf, #fcount, "/", DIRS_ONLYREAL);

	for (i=0; i<fcount; i++)
	{
		strcpy(#drive_name, "/");
		strcat(#drive_name, i*304+dirbuf+72);
		if (!strcmp(#drive_name, "/fd")) continue;
		free(dirbuf2);
		GetDir(#dirbuf2, #fcount2, #drive_name, DIRS_ONLYREAL);

		for (j=0; j<fcount2; j++)
		{
			sprintf(#install_path, "%s/%s/installer.kex", #drive_name, j*304+dirbuf2+72);
			if (RunProgram(#install_path, NULL) > 0) ExitProcess();
		}
	}
	notify("'KolibriN\nНе могу найти installer.kex ни в одном корне диска!\nПопробуйте найти и запустить его вручную.' -dtE");
	ExitProcess();
}


stop:

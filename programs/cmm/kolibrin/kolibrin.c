#define MEMSIZE 0x9000
#include "..\lib\kolibri.h" 
#include "..\lib\strings.h" 
#include "..\lib\mem.h"
#include "..\lib\file_system.h"


void main()
{   
	dword dirbuf, fcount, filename, i;
	dword dirbuf2, fcount2, filename2, j;
	char cd_path[4096];
	char install_path[4096];
	signed int result;

	mem_Init();
	GetDir(#dirbuf, #fcount, "/");

	for (i=0; i<fcount; i++)
	{
		filename = i*304+dirbuf+72;
		if (!strstr(filename, "fd"))
		{
			strcpy(#cd_path, "/");
			strcat(#cd_path, filename);
			free(dirbuf2);
			GetDir(#dirbuf2, #fcount2, #cd_path);

			for (j=0; j<fcount2; j++)
			{
				filename2 = j*304+dirbuf2+72;
				strcpy(#install_path, #cd_path);
				strcat(#install_path, "/");
				strcat(#install_path, filename2);
				strcat(#install_path, "/installer.kex");
				result = RunProgram(#install_path, NULL);
				if (result>0) ExitProcess();
			}
		}
	}
	if (GetSystemLanguage()==4) notify("Не могу найти installer.kex ни в одном корне диска! Попробуйте найти и запустить его вручную.");
	else notify("Can't find installer.kex at the root of all disks! Try to find and run it manually."w);
	ExitProcess();
}


stop:

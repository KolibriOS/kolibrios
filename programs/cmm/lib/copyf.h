//copyf - copy file or folder with content
#ifndef INCLUDE_COPYF_H
#define INCLUDE_COPYF_H
#print "[include <copyf.h>]\n"

#ifndef INCLUDE_FILESYSTEM_H
#include "../lib/fs.h"
#endif

:int copyf(dword from1, in1)
{
	dword error;
	BDVK CopyFile_atr1;

	if (!from1) || (!in1)
	{
		notify("Error: too few copyf() params!");
		return;
	}
	if (error = GetFileInfo(from1, #CopyFile_atr1))
	{
		debugln("Error: copyf->GetFileInfo");
		return error;
	}
	if (dir_exists(from1))
		return CopyFolder(from1, in1);
	else
	{
		Operation_Draw_Progress(from1+strrchr(from1, '/'));
		return CopyFile(from1, in1);
	}
}

:int CopyFile(dword copy_from3, copy_in3)
{
	BDVK CopyFile_atr;
	dword error, cbuf;
	dword block;

	if (error = GetFileInfo(copy_from3, #CopyFile_atr))
	{
		debugln("Error: CopyFile->GetFileInfo");
	}
	else 
	{
		if (GetFreeRAM()-1024*1024 < CopyFile_atr.sizelo) //GetFreeRam-1Mb and convert to bytes
		{
			if (error = CopyFileByBlocks(CopyFile_atr.sizelo, copy_from3, copy_in3))
				debugln("Error: CopyFile->CopyFileByBlocks");
		}
		else {
			if (error = CopyFileAtOnce(CopyFile_atr.sizelo, copy_from3, copy_in3))
				debugln("Error: CopyFile->CopyFileAtOnce");
		}		
	}
	if (error) debug_error(copy_from3, error);
	return error;
}

:int CopyFolder(dword from2, in2)
{
	dword dirbuf, fcount, i, filename;
	char copy_from2[4096], copy_in2[4096], error;

	if (error = GetDir(#dirbuf, #fcount, from2, DIRS_ONLYREAL))
	{
		debugln("Error: CopyFolder->GetDir");
		debug_error(from2, error);
		free(dirbuf);
		return error;
	}

	if (chrnum(in2, '/')>2) && (error = CreateDir(in2))
	{
		debugln("Error: CopyFolder->CreateDir");
		debug_error(in2, error);
		free(dirbuf);
		return error;
	}

	for (i=0; i<fcount; i++)
	{
		filename = i*304+dirbuf+72;
		sprintf(#copy_from2,"%s/%s",from2,filename);
		sprintf(#copy_in2,"%s/%s",in2,filename);

		if ( TestBit(ESDWORD[filename-40], 4) ) //dir_exists?
		{
			if ( (!strncmp(filename, ".",1)) || (!strncmp(filename, "..",2)) ) continue;
			CopyFolder(#copy_from2, #copy_in2);
		}
		else
		{
			Operation_Draw_Progress(filename+strrchr(filename, '/'));
			if (error=CopyFile(#copy_from2, #copy_in2))
			{
				if (fabs(error)==8) { debugln("Stop copying."); break;} //TODO: may be need grobal var like stop_all
				error=CopyFile(#copy_from2, #copy_in2); // #2 :)
			}
		}
	}
	free(dirbuf);
	return error;
}

#ifdef LANG_RUS
	:unsigned char *ERROR_TEXT[]={
	"Код #0: успешно",
	"Ошибка #1: не определена база и/или раздел жёсткого диска",
	"Ошибка #2: функция не поддерживается для этой файловой системы",
	"Ошибка #3: неизвестная файловая система",
	0,
	"Ошибка #5: файл или папка не найдены",
	"Ошибка #6: конец файла",
	"Ошибка #7: указатель находится все памяти приложения",
	"Ошибка #8: недостаточно места на разделе",
	"Ошибка #9: ошибка файловой системы",
	"Ошибка #10: доступ запрещен",
	"Ошибка #11: ошибка устройства",
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Ошибка #30: недостаточно памяти",
	"Ошибка #31: файл не является исполняемым",
	"Ошибка #32: слишком много процессов", 0};
#else
	:unsigned char *ERROR_TEXT[]={
	"Code #0 - No error, compleated successfully",
	"Error #1 - Base or partition of a hard disk is not defined",
	"Error #2 - Function isn't supported for this file system",
	"Error #3 - Unknown file system",
	0,
	"Error #5 - File or folder not found",
	"Error #6 - End of file",
	"Error #7 - Pointer lies outside of application memory",
	"Error #8 - Not enough space on partition",
	"Error #9 - File system error",
	"Error #10 - Access denied",
	"Error #11 - Device error",
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	"Error #30 - Not enough memory",
	"Error #31 - File is not executable",
	"Error #32 - Too many processes", 0};
#endif

:dword get_error(int N)
{
	char error[256];
	N = fabs(N);
	if (N<=33) strcpy(#error, ERROR_TEXT[N]);
	else sprintf(#error,"%d%s",N," - Unknown error number O_o");
	return #error;
}

:void debug_error(dword path, error_number)
{
	if (path) debugln(path);
	debugln(get_error(error_number));
}

#endif
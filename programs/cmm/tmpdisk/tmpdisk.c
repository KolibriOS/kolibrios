#include "..\lib\kolibri.h" 
#include "..\lib\strings.h" 
#include "..\lib\file_system.h"

#include "lang.h"

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////             область данных                 ////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

struct ioctl_struct
{
	dword	handle;
	dword	io_code;
	dword	input;
	dword	inp_size;
	dword	output;
	dword	out_size;
};

#define DEV_ADD_DISK 1	//input = structure add_disk_struc
#define DEV_DEL_DISK 2	//input = structure del_disk_struc

struct add_disk_struc
{
	dword DiskSize; // in sectors, 1 sector = 512 bytes. Include FAT service data
	unsigned char DiskId; // from 0 to 9
};

struct del_disk_struc
{
	unsigned char DiskId; //from 0 to 9
};


ioctl_struct ioctl;
add_disk_struc add_disk;
del_disk_struc del_disk;

int driver_handle;

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////                    код                     ////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
#include "t_console.c"
#include "t_window.c"


void main()
{   
	debug("=========  tmpdisk 0.4  =========");
	driver_handle = LoadDriver("tmpdisk");
	if (driver_handle==0)
	{
		notify("error: /rd1/1/lib/tmpdisk.obj driver loading failed");
		notify("program terminated");
		ExitProcess();
	}
	else
		debug("tmpdisk.obj driver loaded successfully");
	
	if (param)
		Console_Work();
	else
		Main_Window();
		
	
	ExitProcess();
}

stop:

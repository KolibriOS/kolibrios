#define MEMSIZE 0xA0000
#include "..\lib\kolibri.h" 
#include "..\lib\mem.h" 
#include "..\lib\strings.h"
#include "..\lib\fs.h"

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////              Program data                  ////////////////////
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
/////////////////////////                    Code                    ////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
#include "t_console.c"
#include "t_gui.c"


void main()
{   
	if (! driver_handle = LoadDriver("tmpdisk"))
	{
		notify("'TmpDisk\nError: /sys/drivers/tmpdisk.obj driver loading failed\nVirtual disk wouldn't be added' -tE");
		ExitProcess();
	}

	if (param)
		Console_Work();
	else
		Main_Window();
		
	ExitProcess();
}

stop:

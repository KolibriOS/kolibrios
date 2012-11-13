#include "..\lib\kolibri.h" 
#include "..\lib\strings.h" 
#include "..\lib\file_system.h"


//область данных
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


void main()
{   
	int driver_handle;
	int del_st, add_st, size_st;
	
	debug("===  tmpdisk 0.1  ===");
	driver_handle = LoadDriver("tmpdisk");
	if (driver_handle==0) {
		debug("tmpdisk: driver loading failed");
		ExitProcess();
	}
	else
		debug("tmpdisk: driver loaded successfully");
	
	if (!param)
	{
		debug("tmpdisk error: no command line parameters");
		debug("a[number]s[size in MB] - add RAM disk");
		debug("d[number] - delete RAM disk");
		ExitProcess();
	}
	strlwr(#param);
	
	//удалить диск
	del_st=strchr(#param, 'd');
	if (del_st)
	{
		del_disk.DiskId = atoi(#param+del_st);
		ioctl.handle   = driver_handle;
		
		ioctl.io_code  = DEV_DEL_DISK;
		ioctl.input    = #del_disk;
		ioctl.inp_size = sizeof(del_disk);
		ioctl.output   = 0;
		ioctl.out_size = 0;
	}
	
	//добавить диск
	add_st=strchr(#param, 'a');
	if (add_st)
	{
		size_st= strchr(#param, 's');
		if (!size_st)
		{
			debug("tmpdisk: disk size is not specified");
			debug("tmpdisk: DiskSize = 200 MB (by default)");
			add_disk.DiskSize = 200*2048;
		}
		else
			add_disk.DiskSize = atoi(#param+size_st)*2048;
		
		//if (add_disk.DiskSize>GetFreeRam()) 
		
		param[size_st-1]=NULL;
		add_disk.DiskId = atoi(#param+add_st);
		
		ioctl.handle   = driver_handle;
		ioctl.io_code  = DEV_ADD_DISK;
		ioctl.input    = #add_disk;
		ioctl.inp_size = sizeof(add_disk);
		ioctl.output   = 0;
		ioctl.out_size = 0;
	}
	
	if (!ioctl.io_code)
	{
		debug("tmpdisk: unknown command line parameters");
		ExitProcess();
	}
	
	RuleDriver(#ioctl);
	switch(EAX)
	{
		case 0:
			if (ioctl.io_code==DEV_ADD_DISK) debug("tmpdisk: disk added successfully");
			else debug("tmpdisk: disk deleted successfully");
			break;
		case 1:
			debug("tmpdisk: unknown IOCTL code, wrong input/output size...");
			break;
		case 2:
			debug("tmpdisk: DiskId must be from 0 to 9");
			break;
		case 3:
			debug("tmpdisk: DiskSize is too large");
			break;
		case 4:
			debug("tmpdisk: DiskSize is too small");
			break;
		case 5:
			debug("tmpdisk: memory allocation failed");
			break;
		default:
			debug("tmpdisk: unknown error O_o");
	}
	ExitProcess();
}


stop:

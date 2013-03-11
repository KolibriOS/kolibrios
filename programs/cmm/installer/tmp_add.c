struct ioctl_struct
{
	dword	handle;
	dword	io_code;
	dword	input;
	dword	inp_size;
	dword	output;
	dword	out_size;
};

struct add_disk_struc
{
	dword DiskSize; // in sectors, 1 sector = 512 bytes. Include FAT service data
	unsigned char DiskId; // from 0 to 9
};

ioctl_struct ioctl;
add_disk_struc add_disk;

int TmpDiskAdd(int disk_id, disk_size)
{
	int driver_handle, driver_rezult;   
	driver_handle = LoadDriver("tmpdisk");
	if (driver_handle==0) return 7;
	
	add_disk.DiskId = disk_id;
	add_disk.DiskSize = disk_size * 2048;

	ioctl.handle   = driver_handle;
	ioctl.io_code  = 1;
	ioctl.input    = #add_disk;
	ioctl.inp_size = sizeof(add_disk);
	ioctl.output   = 0;
	ioctl.out_size = 0;
	
	driver_rezult = RuleDriver(#ioctl);
	return driver_rezult;
}
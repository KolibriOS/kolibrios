/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////                 Console                    ////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

#ifdef LANG_RUS
	?define DELETE_DISK_TEXT "Пробую удалить /tmp%i"
	?define NEW_DISK_TEXT "Пробую добавить виртуальный диск /tmp%i размером %i MB"
	char *rezult_text[]={
	"TmpDisk операция успешно завершена",
	"неизвестный IOCTL, неверный размер предоставляемых данных...",
	"номер диска должен быть от 0 до 9",
	"размер создаваемого диска слишком велик",
	"размер создаваемого диска слишком мал",
	"ошибка выделения памяти",
	"неизвестная ошибка O_o",
	0};
#else
	?define DELETE_DISK_TEXT "Trying to delete /tmp%i"
	?define NEW_DISK_TEXT "Trying to add virtual disk /tmp%i, the size of %i MB"
	char *rezult_text[]={
	"TmpDisk operation completed successfully",
	"unknown IOCTL code, wrong input/output size...",
	"DiskId must be from 0 to 9",
	"DiskSize is too large",
	"DiskSize is too small, might be too little free RAM",
	"memory allocation failed",
	"unknown error O_o",
	0};
#endif

char Console_Work()
{
	unsigned int disk_size, driver_rezult;
	char size_t[256];

	strlwr(#param);
	
	switch (param[0])
	{
		case '?':
		case 'h':
			debugln("tmpdisk command line parameters:");
			debugln("a[number]s[size in MB] - add RAM disk");
			debugln("d[number] - delete RAM disk");
			ExitProcess();
			break;
		case 'd': //Delete disk
			del_disk.DiskId = param[1]-'0';
			ioctl.handle   = driver_handle;
			ioctl.io_code  = DEV_DEL_DISK;
			ioctl.input    = #del_disk;
			ioctl.inp_size = sizeof(del_disk);
			ioctl.output   = 0;
			ioctl.out_size = 0;
			sprintf(#size_t, DELETE_DISK_TEXT, add_disk.DiskId);
			debugln(#size_t);
			break;
		case 'a': //Add disk
			disk_size= strchr(#param, 's');
			if (!disk_size)
			{
				add_disk.DiskSize = GetFreeRAM() / 5 * 2;
			}				
			else
			{
				add_disk.DiskSize = atoi(disk_size+1)*2048;
			}
			add_disk.DiskId = param[1]-'0';
			ioctl.handle   = driver_handle;
			ioctl.io_code  = DEV_ADD_DISK;
			ioctl.input    = #add_disk;
			ioctl.inp_size = sizeof(add_disk);
			ioctl.output   = 0;
			ioctl.out_size = 0;
			sprintf(#size_t, NEW_DISK_TEXT, add_disk.DiskId, add_disk.DiskSize/2048);
			debugln(#size_t);
			break;
		default:
			debugln("unknown command line parameters");
			debugln("use 'h' or '?' for help");
			ExitProcess();			
	}
	
	driver_rezult = RuleDriver(#ioctl);
	if (driver_rezult<7) debugln(rezult_text[driver_rezult]);
	return driver_rezult;
}


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////                 Console                    ////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

#ifdef LANG_RUS
	?define ADD_DISK_TEXT "пробую добавить виртуальный диск"
	?define DELETE_DISK_TEXT "пробую удалить виртуальный диск"
	?define DONT_KNOW_DISK_SIZE_TEXT "его размер не указан, 20% свободной ОЗУ будет использовано"
	?define NEW_DISK_TEXT "размер диска будет: "
	char *rezult_text[]={
	"операция успешно завершена",
	"неизвестный IOCTL, неверный размер предоставляемых данных...",
	"номер диска должен быть от 0 до 9",
	"размер создаваемого диска слишком велик",
	"размер создаваемого диска слишком мал",
	"ошибка выделения памяти",
	"неизвестная ошибка O_o",
	0};
#else
	?define ADD_DISK_TEXT "trying to add disk"
	?define DELETE_DISK_TEXT "trying to delete virtual disk"
	?define DONT_KNOW_DISK_SIZE_TEXT "its size is not specified, 20% from free RAM will be used"
	?define NEW_DISK_TEXT "new DiskSize: "
	char *rezult_text[]={
	"operation completed successfully",
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
		case 'd':
			debugln(DELETE_DISK_TEXT);
			del_disk.DiskId = param[1]-'0';
			ioctl.handle   = driver_handle;
			ioctl.io_code  = DEV_DEL_DISK;
			ioctl.input    = #del_disk;
			ioctl.inp_size = sizeof(del_disk);
			ioctl.output   = 0;
			ioctl.out_size = 0;
			disk_sizes[del_disk.DiskId] = 0;
			break;
		case 'a':
			debugln(ADD_DISK_TEXT);
			disk_size= strchr(#param, 's');
			if (!disk_size)
			{
				add_disk.DiskSize = GetFreeRAM() / 5 * 2;
				debugln(DONT_KNOW_DISK_SIZE_TEXT);
			}				
			else
			{
				add_disk.DiskSize = atoi(disk_size+1)*2048;
			}
			strcpy(#size_t, NEW_DISK_TEXT);
			strcat(#size_t, itoa(add_disk.DiskSize/2048));
			strcat(#size_t, " MB");
			debugln(#size_t);
			add_disk.DiskId = param[1]-'0';
			ioctl.handle   = driver_handle;
			ioctl.io_code  = DEV_ADD_DISK;
			ioctl.input    = #add_disk;
			ioctl.inp_size = sizeof(add_disk);
			ioctl.output   = 0;
			ioctl.out_size = 0;
			disk_sizes[add_disk.DiskId] = add_disk.DiskSize * 512;
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


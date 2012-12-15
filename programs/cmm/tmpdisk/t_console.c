/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////                консоль                     ////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

/*#ifdef LANG_RUS
	char *rezult_text[]={
	"операция успешно завершена"w,
	"неизвестный IOCTL, неверный размер предоставляемых данных..."w,
	"номер диска должен быть от 0 до 9"w,
	"размер создаваемого диска слишком велик"w,
	"размер создаваемого диска слишком мал"w,
	"ошибка выделения памяти"w,
	"неизвестная ошибка O_o"w,
	0};
#else*/
	char *rezult_text[]={
	"operation compleated successfully",
	"unknown IOCTL code, wrong input/output size...",
	"DiskId must be from 0 to 9",
	"DiskSize is too large",
	"DiskSize is too small",
	"memory allocation failed",
	"unknown error O_o",
	0};
//#endif

void Console_Work()
{
	unsigned int disk_size, driver_rezult;
	char size_t[256];

	strlwr(#param);
	
	switch (param[0])
	{
		case '?': //помощь по коммандам
		case 'h':
			debug("tmpdisk command line parameters:");
			debug("a[number]s[size in MB] - add RAM disk");
			debug("d[number] - delete RAM disk");
			ExitProcess();
			break;
		case 'd': //удалить диск
			debug("trying to delete disk");
			del_disk.DiskId = param[1]-'0';
			ioctl.handle   = driver_handle;
			ioctl.io_code  = DEV_DEL_DISK;
			ioctl.input    = #del_disk;
			ioctl.inp_size = sizeof(del_disk);
			ioctl.output   = 0;
			ioctl.out_size = 0;
			break;
		case 'a': //добавить диск
			debug("trying to add disk");
			disk_size= strchr(#param, 's');
			if (!disk_size)
			{
				add_disk.DiskSize = GetFreeRAM() / 5;
				debug("disk size is not specified");
				strcpy(#size_t, "10% from free RAM will be used, new DiskSize: ");
				strcat(#size_t, itoa(add_disk.DiskSize/2048));
				strcat(#size_t, " MB");
				debug(#size_t);
			}				
			else
				add_disk.DiskSize = atoi(#param+disk_size)*2048;

			add_disk.DiskId = param[1]-'0';
			ioctl.handle   = driver_handle;
			ioctl.io_code  = DEV_ADD_DISK;
			ioctl.input    = #add_disk;
			ioctl.inp_size = sizeof(add_disk);
			ioctl.output   = 0;
			ioctl.out_size = 0;
			break;
		default:
			debug("unknown command line parameters");
			debug("use 'h' or '?' for help");
			ExitProcess();			
	}
	
	driver_rezult = RuleDriver(#ioctl);
	if (driver_rezult<7) debug(rezult_text[driver_rezult]);
}


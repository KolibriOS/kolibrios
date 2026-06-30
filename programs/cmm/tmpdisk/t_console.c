/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////                 Console                    ////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

#ifdef LANG_RUS
	?define DELETE_DISK_TEXT "Пробую удалить /tmp%i"
	?define NEW_DISK_TEXT "Пробую добавить виртуальный диск /tmp%i размером %i MB"
	char *rezult_text[]={
	"'TmpDisk операция успешно завершена' -O",
	"'Неизвестный IOCTL, неверный размер предоставляемых данных...' -E",
	"'Номер диска должен быть от 0 до 9' -E",
	"'Размер создаваемого диска слишком велик' -E",
	"'Размер создаваемого диска слишком мал' -E",
	"'Ошибка выделения памяти' -E",
	"'Неизвестная ошибка O_o' -E",
	0};
#else
#ifdef LANG_SPA
	?define DELETE_DISK_TEXT "Intentando eliminar /tmp%i"
	?define NEW_DISK_TEXT "Intentando anadir disco RAM /tmp%i, de %i MB"
	char *rezult_text[]={
	"'Operacion TmpDisk completada con exito' -O",
	"'Codigo IOCTL desconocido, tamano de E/S incorrecto...' -E",
	"'El numero de disco debe ser de 0 a 9' -E",
	"'El tamano del disco es demasiado grande' -E",
	"'El tamano del disco es demasiado pequeno, quiza falta RAM libre' -E",
	"'Fallo la asignacion de memoria' -E",
	"'Error desconocido O_o' -E",
	0};
#else
	?define DELETE_DISK_TEXT "Trying to delete /tmp%i"
	?define NEW_DISK_TEXT "Trying to add RAM disk /tmp%i, the size of %i MB"
	char *rezult_text[]={
	"'TmpDisk operation completed successfully' -O",
	"'Unknown IOCTL code, wrong input/output size...' -E",
	"'Disk number must be from 0 to 9' -E",
	"'Disk size is too large' -E",
	"'Disk size is too small, might be too little free RAM' -E",
	"'Memory allocation failed' -E",
	"'Unknown error O_o' -E",
	0};
#endif
#endif

char Console_Work()
{
	unsigned int disk_size, driver_rezult;
	char t_size[256];

	strlwr(#param);
	
	switch (param[0])
	{
		case 'd': //Delete disk
			del_disk.DiskId = param[1]-'0';
			ioctl.handle   = driver_handle;
			ioctl.io_code  = DEV_DEL_DISK;
			ioctl.input    = #del_disk;
			ioctl.inp_size = sizeof(del_disk);
			ioctl.output   = 0;
			ioctl.out_size = 0;
			miniprintf(#t_size, DELETE_DISK_TEXT, add_disk.DiskId);
			debugln(#t_size);
			break;
		case 'a': //Add disk
			disk_size= strchr(#param, 's');
			if (!disk_size)	{
				add_disk.DiskSize = GetFreeRAM() / 5 * 2;
			} else {
				add_disk.DiskSize = atoi(disk_size+1)*2048;
			}
			add_disk.DiskId = param[1]-'0';
			ioctl.handle   = driver_handle;
			ioctl.io_code  = DEV_ADD_DISK;
			ioctl.input    = #add_disk;
			ioctl.inp_size = sizeof(add_disk);
			ioctl.output   = 0;
			ioctl.out_size = 0;
			miniprintf(#t_size, NEW_DISK_TEXT, add_disk.DiskId);
			miniprintf(#param, #t_size, add_disk.DiskSize/2048);
			debugln(#param);
			break;
		default:
			debugln("\ntmpdisk: unknown command line parameters!");
			debugln("valid parameters are:");
			debugln("a[number]s[size in MB] - add RAM disk");
			debugln("d[number] - delete RAM disk");
			ExitProcess();
	}
	
	driver_rezult = RuleDriver(#ioctl);
	if (driver_rezult<7) debugln(rezult_text[driver_rezult]);
	return driver_rezult;
}



int cmd_more(char file[])
{

kol_struct70	k70;
kol_struct_BDVK	bdvk;
unsigned	result, filesize, pos, i;
char		buf[81]; //буфер
char		temp[256];
unsigned	flags;

if (strlen(file)<1)
	{
	#if LANG_ENG
		printf ("  more <filename>\n\r");
	#elif LANG_RUS
		printf ("  more <имя файла>\n\r");
	#endif
	return TRUE;
	}

if ( '/' == file[0])
	{
	strcpy(temp, file);

	if ( !file_check(temp) )
		{
		file_not_found(file);
		return FALSE;
		}
	}
else
	{
	strcpy(temp, cur_dir);
	if (temp[strlen(temp)-1] != '/') 
		strcat(temp, "/"); // add slash
	strcat(temp, file);
	
	if ( !file_check(temp) )
		{
		file_not_found(file);
		return FALSE;
		}
	}

k70.p00 = 5;
k70.p04 = k70.p08 = k70.p12 = 0;
k70.p16 = (unsigned) &bdvk;
k70.p20 = 0;
k70.p21 = temp;

result = kol_file_70(&k70); // получаем информацию о файле
if ( 0 != result ) 
	return FALSE;

filesize = bdvk.p32[0]; // получаем размер файла

buf[80]=0;
flags = con_get_flags();

for (pos=0;pos<filesize;pos+=80)
	{

	memset(buf, 0, 80);

	k70.p00 = 0;
	k70.p04 = pos;
	k70.p08 = 0;
	k70.p12 = 80;
	k70.p16 = (unsigned) buf;
	k70.p20 = 0;
	k70.p21 = temp;

	result = kol_file_70(&k70); // чтение 80 символов
	for (i=0; i<80; i++)
		{

		if (27 == buf[i])
			con_set_flags(flags|0x100);
		else con_set_flags(flags);

		printf ("%c", buf[i]);
		}
	if ( 0 != result ) 
		{
		con_set_flags(flags);
		printf ("\n\r");
		return TRUE;	
		}

	}
con_set_flags(flags);
printf ("\n\r");
return TRUE;
}


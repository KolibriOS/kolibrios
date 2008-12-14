
int cmd_ls(char dir[])
{

kol_struct70	k70;
unsigned	*n;
unsigned	num_of_file;
unsigned	*t;
unsigned	type_of_file;
int		i;

k70.p00 = 1;
k70.p04 = 0;
k70.p08 = 0;
k70.p12 = 2*1024*1024; // 2 MB
k70.p16 = malloc(2*1024*1024);
k70.p20 = 0;
k70.p21 = dir;

if ( !kol_file_70(&k70) ) // проверяем существование каталога
	{
	free(k70.p16);
	return FALSE;
	}

n = k70.p16+8;
num_of_file = *n; // число файлов в каталоге

for (i = 0; i < num_of_file; i++)
	{
	printf ("  %s", k70.p16+32+40+(264+40)*i);
	t = k70.p16+32+(264+40)*i;
	type_of_file = *t;
	if ( (0x10 == (type_of_file&0x10)) || (8 == (type_of_file&8)) )
		printf ("/");
	printf ("\n\r");
	}

free(k70.p16);
return TRUE;
}

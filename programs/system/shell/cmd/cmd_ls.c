
int cmd_ls(char dir[])
{

kol_struct70	k70;
unsigned	*n;
unsigned	num_of_file;
unsigned	*t;
unsigned	type_of_file;
int		i, result;


k70.p00 = 1;
k70.p04 = 0;
k70.p08 = 0;
k70.p12 = 10000; 
k70.p16 =  (unsigned) malloc(32+k70.p12*560);
k70.p20 = 0;

/// !!!
// Если ls запускается без параметров, просматриваем текущий каталог
if ( !strlen(dir) )
	k70.p21 = cur_dir;
else
	k70.p21 = dir;

result = kol_file_70(&k70);
if ( !((result==0) || (result==6)) ) // проверяем существование каталога
	{
	free( (void*) k70.p16);
	return FALSE;
	}

n =  (unsigned*) (k70.p16+8);
num_of_file = *n; // число файлов в каталоге

for (i = 0; i < num_of_file; i++)
	{
	printf ("  %s", k70.p16+32+40+(264+40)*i);
	t =  (unsigned*) (k70.p16+32+(264+40)*i);
	type_of_file = *t;
	if ( (0x10 == (type_of_file&0x10)) || (8 == (type_of_file&8)) )
		printf ("/");
	printf ("\n\r");
	}

free((void*)k70.p16);
return TRUE;
}


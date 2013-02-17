
int cmd_cp(char param[])
{

char* argv[100];
int i;
int argc;
char *filename_in;
char *filename_out;
char *buffer;

kol_struct70	k70_in;
kol_struct70	k70_out;

kol_struct_BDVK	bdvk;

unsigned filesize, result;
unsigned n; // количество раз копирования по 4 кбайта

argc = parameters_prepare(param, argv);

if (argc != 2)
   {
      #if LANG_ENG
	printf("  cp <file_in> <file_out>\n\r");
      #elif LANG_RUS
	printf("  cp <источник> <результат>\n\r");
      #endif

      parameters_free(argc, argv);

      return TRUE;
   }

filename_in  = (char*) malloc(4096);
filename_out = (char*) malloc(4096);

if (argv[0][0] != '/')
   {
   strcpy(filename_in, cur_dir);
   if (filename_in[strlen(filename_in)-1] != '/')
	strcat(filename_in, "/"); // add slash
   strcat(filename_in, argv[0]);
   }
   else
   {
   strcpy(filename_in, argv[0]);
   }

if (argv[1][0] != '/')
   {
   strcpy(filename_out, cur_dir);
   if (filename_out[strlen(filename_out)-1] != '/')
	strcat(filename_out, "/"); // add slash
   strcat(filename_out, argv[1]);
   }
   else
   {
   strcpy(filename_out, argv[1]);
   }


k70_in.p00 = 5;
k70_in.p04 = k70_in.p08 = k70_in.p12 = 0;
k70_in.p16 = (unsigned) &bdvk;
k70_in.p20 = 0;
k70_in.p21 = filename_in;

result = kol_file_70(&k70_in); // получаем информацию о файле
if ( 0 != result )
       {
       parameters_free(argc, argv);
       free(filename_in);
       free(filename_out);
       return TRUE;
       }

filesize = bdvk.p32[0]; // получаем размер файла  (ограничение - 4 Гбайта)
n = filesize / 4096;

buffer = (char*) malloc(4096);

k70_in.p00 = 0;
k70_in.p08 = 0;
k70_in.p12 = 4096;
k70_in.p16 = (unsigned) buffer;
k70_in.p20 = 0;
k70_in.p21 = filename_in;

k70_out.p00 = 2;
k70_out.p08 = 0;
k70_out.p12 = 4096;
k70_out.p16 = (unsigned) buffer;
k70_out.p20 = 0;
k70_out.p21 = filename_out;

i = 0; // для того, чтобы копировать файлы с размером меньше 4 кБайт
for ( i = 0; i < n; i++)
    {

    k70_in.p04 = i*4096;
    result = kol_file_70(&k70_in); // чтение
    if (result != 0)
       {
       parameters_free(argc, argv);
       free(filename_in);
       free(filename_out);
       free(buffer);
       return FALSE;
       }

    k70_out.p04 = i*4096;
    result = kol_file_70(&k70_out); // запись
    if (result != 0)
       {
       parameters_free(argc, argv);
       free(filename_in);
       free(filename_out);
       free(buffer);
       return FALSE;
       }

    if (i == 0)
       k70_out.p00 = 3; // меняем функцию с создания (2) на дозапись (3)

    }

if ( (filesize%4096) != 0 ) // если размер файла не кратен 4 кБайтам
   {

   k70_in.p12  = filesize%4096;
   k70_out.p12 = filesize%4096;

   k70_in.p04 = i*4096; // в i должно быть правильное смещение
   result = kol_file_70(&k70_in); // чтение
   if (result != 0)
       {
       parameters_free(argc, argv);
       free(filename_in);
       free(filename_out);
       free(buffer);
       return FALSE;
       }

    k70_out.p04 = i*4096;
    result = kol_file_70(&k70_out); // запись
    if (result != 0)
       {
       parameters_free(argc, argv);
       free(filename_in);
       free(filename_out);
       free(buffer);
       return FALSE;
       }

   }


parameters_free(argc, argv);
free(filename_in);
free(filename_out);
free(buffer);


return TRUE;
}


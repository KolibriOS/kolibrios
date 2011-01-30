
#include <stdio.h>

const char hextable[] = "0123456789abcdef";

int main(int argc, char *argv[])
{

FILE *fin;
FILE *fout;
unsigned i;
int num;

if (argc != 3)
	{
	printf ("file2c v0.2 by O.Bogomaz (albom85@yandex.ru)\nfile2c.exe file.in file.out\n\n");
	return -1;
	}

fin = fopen (argv[1], "rb");
fout = fopen (argv[2], "wt");

fprintf(fout, "char array[]= {");

for (i = 0;; i++)
	{
	if ( 0 == i%16)
		fprintf(fout, "\n");
	num = getc(fin);
	if (feof(fin))
		break;
	fprintf (fout, "0x%c%c,", (int)hextable[(num >> 4)], (int)hextable[(num & 0x0f)]);
	}

fprintf(fout, "\n };\n");

return 0;
}

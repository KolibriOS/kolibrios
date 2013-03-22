
#include "kolibri.h"
#include "stdlib.h"
#include "string.h"

#include "console.c"


void (* __stdcall qsi)(int* d, int n);

/// ===========================================================

void kol_main()
{

#define NUM 20000

kol_struct_import *imp_qs;

int	*a;
int	i;

CONSOLE_INIT("Example");

imp_qs = kol_cofflib_load("/sys/lib/qs.obj");
qsi = ( _stdcall void (*)(int*, int))
		kol_cofflib_procload (imp_qs, "qsi");

a = malloc(NUM*sizeof(int));

for (i = 0; i < NUM; i++)
	*(a+i) = random(10000);

for (i = 0; i < 5; i++)
	printf("%7d", *(a+i));

printf ("    ...");

for (i = NUM-5; i < NUM; i++)
	printf("%7d", *(a+i));

qsi(a, NUM);

printf ("\n");

for (i = 0; i < 5; i++)
	printf("%7d", *(a+i));

printf ("    ...");

for (i = NUM-5; i < NUM; i++)
	printf("%7d", *(a+i));


free(a);

_exit(0);
kol_exit();
}

/// ===========================================================

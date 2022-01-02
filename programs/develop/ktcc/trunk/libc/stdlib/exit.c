#include <stdlib.h>
#include <conio.h>
#include <kolibrisys.h>

void exit (int status)
/* close console if was initialized, also stay window [finished] when status is error < 0 */ 
{
    if (__console_initdll_status) 
	con_exit(status > 0);
    _ksys_exit();
}
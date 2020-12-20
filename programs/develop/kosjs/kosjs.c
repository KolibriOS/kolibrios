#include <stdio.h>
#include <stdlib.h>

#include <mujs.h>
#include <import.h>


int main(int argc, char **argv)
{
    import_functions();
    js_dofile(J, argv[1]);
    js_freestate (J);
    exit(0);
} 

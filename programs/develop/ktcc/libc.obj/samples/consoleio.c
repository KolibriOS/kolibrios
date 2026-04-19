// demonstration conio use, color text
// more info in conio.h

#include <conio.h>

int main()
{
    if (con_init())
        return 1; // init fail
    (*con_set_title)("Console colors");

    //   con_write_asciiz("\033[0;31;42m test  \n"); // red on green bk

    for (int i = 30; i < 48; i++) {
        (*con_printf)("\033[%dmColor 0x%02X: ", i, i);
        (*con_write_asciiz)("Text sample.");
        (*con_printf)("  printf %s test %d\n", "small", i);
    }

    (*con_exit)(0);
}
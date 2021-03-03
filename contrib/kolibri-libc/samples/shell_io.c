#include <shell_api.h>
#include <ksys.h>
#include <stdio.h>

int main()
{   
    char string[256];
    shell_cls();
    shell_puts("This is a test console application for Shell\n\r");
    shell_puts("Type a string (255 symbols max): ");
    shell_gets(string);
    shell_puts("You typed:\n\r");
    shell_puts(string);
    shell_puts("Press any key: ");
    string[0] = shell_getc();
    shell_puts("\n\rYou pressed: ");
    shell_putc(string[0]);
    shell_exit();
} 

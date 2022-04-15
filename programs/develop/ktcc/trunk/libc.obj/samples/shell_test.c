#include "../include/shell_api.h"

char string[256];

int main()
{
    shell_cls();
    shell_printf("SHELL PID=%d\n\r", shell_get_pid());

    shell_puts("This is a test console application for Shell\n\r");
    shell_puts("Type a string (255 symbols max): ");

    shell_gets(string, 255);
    shell_printf("You typed: %s\n\r", string);

    shell_puts("Press any key: ");
    string[0] = shell_getc();
    shell_printf("\n\rYou pressed: %c", string[0]);
    shell_exit();
    return 0;
}
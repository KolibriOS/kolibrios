#include <shell_api.h>
#include <string.h>

void shell_puts(const char *str)
{
    shell_write_string(str, strlen(str));
}

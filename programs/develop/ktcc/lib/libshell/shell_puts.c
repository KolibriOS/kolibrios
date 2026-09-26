#include <shell_api.h>
#include <string.h>

size_t shell_puts(const char *str)
{
    return shell_write_string(str, strlen(str));
}

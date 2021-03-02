#include <ksys.h>
#include <sys/dirent.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    DIR *dir;
    struct dirent *entry;
    dir = opendir("/sys");
    if (!dir) {
        _ksys_debug_puts("diropen error!\n");
        return 0;
    };
    while ((entry = readdir(dir)) != NULL) {
        _ksys_debug_puts(entry->d_name);
        _ksys_debug_putc('\n');
    };
    _ksys_debug_puts(itoa(telldir(dir)));
    closedir(dir);
}

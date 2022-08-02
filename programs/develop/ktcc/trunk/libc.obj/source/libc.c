#include <setjmp.h>

#include "libtcc1/libtcc1.c"
#include "ctype/is.c"
#include "ctype/tolower.c"
#include "ctype/toupper.c"

#include "sys/closedir.c"
#include "sys/dir.c"
#include "sys/opendir.c"
#include "sys/readdir.c"
#include "sys/rewinddir.c"
#include "sys/seekdir.c"
#include "sys/socket.c"
#include "sys/telldir.c"

#include "stdio/clearerr.c"
#include "stdio/conio.c"
#include "stdio/debug_printf.c"
#include "stdio/fclose.c"
#include "stdio/feof.c"
#include "stdio/ferror.c"
#include "stdio/fflush.c"
#include "stdio/fgetc.c"
#include "stdio/fgetpos.c"
#include "stdio/fgets.c"
#include "stdio/fopen.c"
#include "stdio/format_print.c"
#include "stdio/fprintf.c"
#include "stdio/fputc.c"
#include "stdio/fputs.c"
#include "stdio/fread.c"
#include "stdio/freopen.c"
#include "stdio/fscanf.c"
#include "stdio/fseek.c"
#include "stdio/fsetpos.c"
#include "stdio/ftell.c"
#include "stdio/fwrite.c"
#include "stdio/getchar.c"
#include "stdio/gets.c"
#include "stdio/perror.c"
#include "stdio/printf.c"
#include "stdio/puts.c"
#include "stdio/remove.c"
#include "stdio/rename.c"
#include "stdio/rewind.c"
#include "stdio/scanf.c"
#include "stdio/setbuf.c"
#include "stdio/setvbuf.c"
#include "stdio/snprintf.c"
#include "stdio/sprintf.c"
#include "stdio/sscanf.c"
#include "stdio/strntoumax.c"
#include "stdio/tmpfile.c"
#include "stdio/tmpnam.c"
#include "stdio/ungetc.c"
#include "stdio/vfprintf.c"
#include "stdio/vprintf.c"
#include "stdio/vsnprintf.c"
#include "stdio/vsscanf.c"

#include "string/memccpy.c"
#include "string/memchr.c"
#include "string/memcmp.c"
#include "string/memcpy.c"
#include "string/strcat.c"
#include "string/strchr.c"
#include "string/strcmp.c"
#include "string/strcoll.c"
#include "string/strcpy.c"
#include "string/strcspn.c"
#include "string/strdup.c"
#include "string/strerror.c"
#include "string/strlen.c"
#include "string/strncat.c"
#include "string/strncmp.c"
#include "string/strncpy.c"
#include "string/strpbrk.c"
#include "string/strrchr.c"
#include "string/strrev.c"
#include "string/strspn.c"
#include "string/strstr.c"
#include "string/strtok.c"
#include "string/strxfrm.c"
#include "stdlib/abs.c"
#include "stdlib/assert.c"
#include "stdlib/atof.c"
#include "stdlib/atoi.c"
#include "stdlib/atol.c"
#include "stdlib/atoll.c"
#include "stdlib/calloc.c"
#include "stdlib/exit.c"
#include "stdlib/free.c"
#include "stdlib/itoa.c"
#include "stdlib/labs.c"
#include "stdlib/llabs.c"
#include "stdlib/malloc.c"
#include "stdlib/qsort.c"
#include "stdlib/rand.c"
#include "stdlib/realloc.c"
#include "stdlib/strtod.c"
#include "stdlib/strtol.c"

#include "math/acosh.c"
#include "math/asinh.c"
#include "math/atanh.c"
#include "math/cosh.c"
#include "math/frexp.c"
#include "math/hypot.c"
#include "math/ldexp.c"
#include "math/sinh.c"
#include "math/tanh.c"

#include "time/asctime.c"
#include "time/difftime.c"
#include "time/localtime.c"
#include "time/mktime.c"
#include "time/time.c"

#include "misc/basename.c"
#include "misc/dirname.c"

ksys_dll_t EXPORTS[] = {
    { "clearerr", &clearerr },
    { "debug_printf", &debug_printf },
    { "fclose", &fclose },
    { "feof", &feof },
    { "ferror", &ferror },
    { "fflush", &fflush },
    { "fgetc", &fgetc },
    { "fgetpos", &fgetpos },
    { "fgets", &fgets },
    { "fopen", &fopen },
    { "fprintf", &fprintf },
    { "fputc", &fputc },
    { "fputs", &fputs },
    { "fread", &fread },
    { "freopen", &freopen },
    { "fscanf", &fscanf },
    { "fseek", &fseek },
    { "fsetpos", &fsetpos },
    { "ftell", &ftell },
    { "fwrite", &fwrite },
    { "getchar", &getchar },
    { "gets", &gets },
    { "perror", &perror },
    { "printf", &printf },
    { "puts", &puts },
    { "remove", &remove },
    { "rename", &rename },
    { "rewind", &rewind },
    { "scanf", &scanf },
    { "setbuf", &setbuf },
    { "setvbuf", &setvbuf },
    { "snprintf", &snprintf },
    { "sprintf", &sprintf },
    { "sscanf", &sscanf },
    { "tmpfile", &tmpfile },
    { "tmpnam", &tmpnam },
    { "vfscanf", &vfscanf },
    { "vprintf", &vprintf },
    { "vfscanf", &vfscanf },
    { "vsprintf", &vsprintf },
    { "vsnprintf", &vsnprintf },
    { "vsscanf", &vsscanf },
    { "ungetc", &ungetc },
    { "abs", &abs },
    { "atoi", &atoi },
    { "atol", &atol },
    { "atoll", &atoll },
    { "atof", &atof },
    { "calloc", &calloc },
    { "exit", &exit },
    { "free", &free },
    { "itoa", &itoa },
    { "labs", &labs },
    { "llabs", &llabs },
    { "malloc", &malloc },
    { "realloc", &realloc },
    { "strtol", &strtol },
    { "srand", &srand },
    { "rand", &rand },
    { "qsort", &qsort },
    { "strtod", &strtod },
    { "__assert_fail", &__assert_fail },
    { "memchr", &memchr },
    { "memcmp", &memcmp },
    { "strncat", &strncat },
    { "strchr", &strchr },
    { "strcat", &strcat },
    { "strcmp", &strcmp },
    { "strcoll", &strcoll },
    { "strcpy", &strcpy },
    { "strcspn", &strcspn },
    { "strdup", &strdup },
    { "strerror", &strerror },
    { "strlen", &strlen },
    { "strncat", &strncat },
    { "strncmp", &strncmp },
    { "strncpy", &strncpy },
    { "strrchr", &strrchr },
    { "strrev", &strrev },
    { "strspn", &strspn },
    { "strstr", &strstr },
    { "strtok", &strtok },
    { "strxfrm", &strxfrm },
    { "__errno", &__errno },
    { "closedir", &closedir },
    { "opendir", &opendir },
    { "readdir", &readdir },
    { "rewinddir", &rewinddir },
    { "seekdir", &seekdir },
    { "telldir", &telldir },
    { "getcwd", &getcwd },
    { "mkdir", &mkdir },
    { "rmdir", &rmdir },
    { "setcwd", &setcwd },
    { "getcwd", &getcwd },
    { "socket", &socket },
    { "close", &close },
    { "bind", &bind },
    { "listen", &listen },
    { "connect", &connect },
    { "accept", &accept },
    { "send", &send },
    { "recv", &recv },
    { "setsockopt", &setsockopt },
    { "socketpair", &socketpair },
    { "acosh", &acosh },
    { "asinh", &asinh },
    { "atanh", &atanh },
    { "frexp", &frexp },
    { "hypot", &hypot },
    { "ldexp", &ldexp },
    { "cosh", &cosh },
    { "sinh", &sinh },
    { "tanh", &tanh },
    { "acos", &acos },
    { "asin", &asin },
    { "atan", &atan },
    { "atan2", &atan2 },
    { "ceil", &ceil },
    { "cos", &cos },
    { "sin", &sin },
    { "tan", &tan },
    { "sqrt", &sqrt },
    { "exp", &exp },
    { "fabs", &fabs },
    { "floor", &floor },
    { "fmod", &fmod },
    { "log", &log },
    { "log2", &log2 },
    { "log10", &log10 },
    { "round", &round },
    { "modf", &modf },
    { "modfl", &modfl },
    { "pow", &pow },
    { "pow2", &pow2 },
    { "pow10", &pow10 },
    { "longjmp", &longjmp },
    { "setjmp", &setjmp },
    { "__is", &__is },
    { "tolower", &tolower },
    { "toupper", &toupper },
    { "con_set_title", &con_set_title },
    { "con_init", &con_init },
    { "con_init_opt", &con_init_opt },
    { "con_write_asciiz", &con_write_asciiz },
    { "con_write_string", &con_write_string },
    { "con_printf", &con_printf },
    { "con_exit", &con_exit },
    { "con_get_flags", &con_get_flags },
    { "con_set_flags", &con_set_flags },
    { "con_kbhit", &con_kbhit },
    { "con_getch", &con_getch },
    { "con_getch2", &con_getch2 },
    { "con_gets", &con_gets },
    { "con_gets2", &con_gets2 },
    { "con_get_font_height", &con_get_font_height },
    { "con_get_cursor_height", &con_get_cursor_height },
    { "con_set_cursor_height", &con_set_cursor_height },
    { "con_cls", &con_cls },
    { "con_get_cursor_pos", &con_get_cursor_pos },
    { "con_set_cursor_pos", &con_set_cursor_pos },
    { "mktime", &mktime },
    { "time", &time },
    { "localtime", &localtime },
    { "asctime", &asctime },
    { "difftime", &difftime },
    { "basename", &basename },
    { "dirname", &dirname },
    NULL
};


#include <drm/drmP.h>
#include <drm/radeon_drm.h>
#include "radeon.h"
#include "radeon_object.h"

extern int radeon_benchmarking;

static int my_atoi(char **cmd)
{
    char* p = *cmd;
    int val = 0;

    for (;; *p++) {
        switch (*p) {
        case '0' ... '9':
            val = 10*val+(*p-'0');
            break;
        default:
            *cmd = p;
            return val;
        }
    }
}

char* parse_mode(char *p, videomode_t *mode)
{
    char c;

    while( (c = *p++) == ' ');

    if( c )
    {
        p--;

        mode->width = my_atoi(&p);
        if(*p == 'x') p++;

        mode->height = my_atoi(&p);
        if(*p == 'x') p++;

        mode->bpp = 32;

        mode->freq = my_atoi(&p);

        if( mode->freq == 0 )
            mode->freq = 60;
    }

    return p;
};

char* parse_path(char *p, char *log)
{
    char  c;

    while( (c = *p++) == ' ');
    p--;
    while( (c = *log++ = *p++) && (c != ' '));
    *log = 0;

    return p;
};

void parse_cmdline(char *cmdline, videomode_t *mode, char *log, int *kms)
{
    char *p = cmdline;

    char c = *p++;

    while( c )
    {
        if( c == '-')
        {
            switch(*p++)
            {
                case 'b':
                    radeon_benchmarking = 1;
                    break;

                case 'l':
                    p = parse_path(p, log);
                    break;

                case 'm':
                    p = parse_mode(p, mode);
                    break;

                case 'n':
                    *kms = 0;
            };
        };
        c = *p++;
    };
};

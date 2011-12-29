
#include "base64.h"

///===============

char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

///===============

char* strchr (const char *s, int c)
{
  do {
    if (*s == c)
      {
        return (char*)s;
      }
  } while (*s++);
  return (0);
}

///===============

int base64_encode(char inp[], char outp[], int len)
{
int i, j;
unsigned char chr[3];

for (i = 0, j=-1; i < len; i+=3)
	{
	chr[0] = (unsigned char) inp[i];
	if (i < len)
		{
		chr[1] = (unsigned char) inp[i+1];
		chr[2] = (unsigned char) inp[i+2];
		}

	outp[++j] = base64_table[ chr[0]>>2 ];
	outp[++j] = base64_table[ ((chr[0] & 3) << 4) | (chr[1] >> 4) ];

	outp[++j] = base64_table[ ((chr[1] & 15) << 2) | (chr[2] >> 6) ];
	outp[++j] = base64_table[ chr[2] & 63 ];
	}

switch (len%3)
    {
    case 1:
            outp[j-1] = outp[j] = '=';
            break;

    case 2:
            outp[j] = '=';
            break;

    default:
        break;

    };

outp[j+1] = '\0';

return j+1;
}

///===============

int base64_decode(char inp[], char outp[], int len)
{
int i, j, k;
char chr[4];

for (i = 0, j=-1, k=0; i < len; i+=4)
    {
    chr[0] = strchr(base64_table, inp[i]) - base64_table;

    chr[1] = strchr(base64_table, inp[i+1]) - base64_table;

    if (inp[i+2] == '=')
        {
        chr[2] = 0;
        k++;
        }
    else
        chr[2] = strchr(base64_table, inp[i+2]) - base64_table;

    if (inp[i+3] == '=')
        {
        chr[3] = 0;
        k++;
        }
    else
        chr[3] = strchr(base64_table, inp[i+3]) - base64_table;

    outp[++j] = ((chr[0] << 2) | (chr[1] >> 4));
    outp[++j] = ((chr[1] << 4) | (chr[2] >> 2));
    outp[++j] = ((chr[2] & 0x03 )<< 6) | (chr[3] & 0x3f);
    }

outp[j+1-k] = '\0';

return j+1-k;
}

///===============

#define NULL ((void*)0)

typedef struct
{
void	*name;
void	*function;
} export_t;

char szbase64_encode[]={"base64_encode"};
char szbase64_decode[]={"base64_decode"};

export_t EXPORTS[] =
{
{szbase64_encode, (void*) base64_encode},
{szbase64_decode, (void*) base64_decode},
{ NULL, NULL },
};

///===============

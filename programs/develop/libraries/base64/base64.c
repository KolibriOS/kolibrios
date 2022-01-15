
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
	int i, j, k, n;
	char *chr_adr;
	char chr[4];

	i = 0;
	j = -1;
	k = 0;
	while (i < len)
	{
		for (n = 0; n <= 3; n++)
		{
			chr[n] = 0;
			while ((inp[i] <= 0x20) & (i < len))
				i++;
			if (i < len)
			{
				if ((n >= 2) & (inp[i] == '='))
					k++;
				else
				{
					chr_adr = strchr(base64_table, inp[i]);
					if (chr_adr)
						chr[n] = chr_adr - base64_table;
				}				
				i++;
			}
		}		
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

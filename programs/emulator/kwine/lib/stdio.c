

int putchar(int ch)
{
	con_init_console_dll();
	char str[2];
	str[0] = ch;
	str[1] = 0;
	con_write_asciiz(str);
	return ch;
}

void puts(const char *str)
{
	con_init_console_dll();
	con_write_asciiz(str);
}

char* gets(char* str)
{
	con_init_console_dll();
	return con_gets(str, 256);
}


void putuint(int i)
{
    unsigned int n, d = 1000000000;
    char str[255];
    unsigned int dec_index = 0;
    while( ( i/d == 0 ) && ( d >= 10 ) ) d /= 10;
    n = i;
    while(d >= 10)
    {
        str[dec_index++] = ((char)((int)'0' + n/d));
        n = n % d;
        d /= 10;
    }
    str[dec_index++] = ((char)((int)'0' + n));
    str[dec_index] = 0;
    puts(str);
}

void putint(int i)
{
    if(i >= 0)
    {
        putuint(i);
    } else {
        putchar('-');
        putuint(-i);
    }
}


void puthex(uint32_t i)
{
    const unsigned char hex[16]  =  { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    unsigned int n, d = 0x10000000;

    puts("0x");
    while((i / d == 0) && (d >= 0x10)) d /= 0x10;
    n = i;
    while( d >= 0xF )
    {
        putchar(hex[n/d]);
        n = n % d;
        d /= 0x10;
    }
    putchar(hex[n]);
}


void print(char *format, va_list args)
{
    int i = 0;
    char *string;

    while (format[i])
    {
        if (format[i] == '%')
        {
            i++;
            switch (format[i])
            {
                case 's':
                    string = va_arg(args, char*);
                    puts(string);
                    break;
                case 'c':
                    // To-Do: fix this! "warning: cast to pointer from integer of different size"
                    putchar(va_arg(args, int));
                    break;
                case 'd':
                    putint(va_arg(args, int));
                    break;
                case 'i':
                    putint(va_arg(args, int));
                    break;
                case 'u':
                    putuint(va_arg(args, unsigned int));
                    break;
                case 'x':
                    puthex(va_arg(args, uint32_t));
                    break;
                default:
                    putchar(format[i]);
            }
        } else {
            putchar(format[i]);
        }
        i++;
    }//endwhile
}


void printf(char *text, ... )
{
    va_list args;
    // find the first argument
    va_start(args, text);
    // pass print the output handle the format text and the first argument
    print(text, args);
}
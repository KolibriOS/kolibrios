
typedef  unsigned char        u8_t;
typedef  unsigned short int   u16_t;
typedef  unsigned int         u32_t;
typedef  unsigned long long   u64_t;

static inline u8_t inb(u16_t port)
{
  u8_t val;
  if(port < 0x100)
    asm volatile ("in %b0, %w1 \n" : "=a" (val) : "dN" (port) );
  else
    asm volatile ("in %b0, %w1 \n" : "=a" (val) : "d" (port) );
	return val;
}

static inline outb(u16_t port, u8_t val)
{
  if (port < 0x100) /* GCC can optimize this if constant */
    asm volatile ("out %w0, %b1" : :"dN"(port), "a"(val));
  else
    asm volatile ("out %w0, %b1" : :"d"(port), "a"(val));
}


/* Convert the integer D to a string and save the string in BUF. If
   BASE is equal to 'd', interpret that D is decimal, and if BASE is
   equal to 'x', interpret that D is hexadecimal.  */
static void itoa (char *buf, int base, int d)
{
  char *p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;

  /* If %d is specified and D is minus, put `-' in the head.  */
  if (base == 'd' && d < 0)
    {
      *p++ = '-';
      buf++;
      ud = -d;
    }
  else if (base == 'x')
    divisor = 16;

  /* Divide UD by DIVISOR until UD == 0.  */
  do
    {
      int remainder = ud % divisor;

      *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    }
  while (ud /= divisor);

  /* Terminate BUF.  */
  *p = 0;

  /* Reverse BUF.  */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2)
    {
      char tmp = *p1;
      *p1 = *p2;
      *p2 = tmp;
      p1++;
      p2--;
    }
}

void putc(int c)
{
    while (!(inb(0x3f8+5) & 0x60));
    outb(0x3f8,c);
    if (c == '\n')
      putc('\r');
}

void _printf (const char *format, ...)
{
  char **arg = (char **) &format;
  int c;
  char buf[20];

  arg++;

  while ((c = *format++) != 0)
  {
    if (c != '%')
      putc(c);
    else
    {
      char *p;

      c = *format++;
      switch (c)
      {
        case 'd':
        case 'u':
        case 'x':
          itoa (buf, c, *((int *) arg++));
          p = buf;
          goto string;
          break;

        case 's':
          p = *arg++;
          if (! p)
            p = "(null)";

  string:
          while (*p)
          putc(*p++);
          break;

        default:
          putc(*((int *) arg++));
          break;
	    }
    }
  }
}


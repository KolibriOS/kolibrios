#include <stdio.h>
#include <stdlib.h>

int fprintf(FILE* file, const char* format, ...)
{
va_list		arg;
char		*buf;
int		printed;
//int		data[4];
	
  va_start (arg, format);
  buf=malloc(4096*2); //8kb max
  //data[0]=(int)&arg-(int)&format;

  printed=format_print(buf,8191, format,arg);
  if (file == stderr)
  	debug_out_str(buf);
  else
  	fwrite(buf,printed,1,file);
  free(buf);

  return(printed);
}

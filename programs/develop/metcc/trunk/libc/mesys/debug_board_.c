#include <mesys.h>
void debug_out_str(char* str)
{
  while ((*str!='\0') || (*str!=0))
  {
    _msys_debug_out(*str);
    str++;
  }
}

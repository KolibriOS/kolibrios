#include <menuet/os.h>
void __menuet__debug_out(const char* str){
  while(*str)
    __menuet__debug_out_byte(*str++);
}

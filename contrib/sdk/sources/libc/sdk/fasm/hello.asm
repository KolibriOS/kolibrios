
STACK_SIZE  equ 4096

include "app.inc"

align 8
main:
          cinvoke _printf, msg_hello
          ret

msg_hello db 'Hello world!',0x0D,0x0A,0

align 16
__idata_start:

  library libc,'libc.dll'

include 'libc.inc'



__idata_end:

__iend:

__cmdline: rb 256
__pgmname: rb 1024
           rb 16
__stack:
__bssend:

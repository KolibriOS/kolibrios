LIBC_BASE    = 0x800145ef
LIBC_SPRINTF = LIBC_BASE+0x1568

assert libc_ptr = LIBC_BASE

libc_ptr: file 'libc.bin'
OUTPUT_FORMAT(binary)

LIBC_BASE = 0x800145ef;

SECTIONS
{
    . = LIBC_BASE;
    .text :
    {
        *(.text);
    }
    .data :
    {
        *(.data);
        *(.bss);
        *(.rodata);
    }
    _heap = ALIGN(4);
} 

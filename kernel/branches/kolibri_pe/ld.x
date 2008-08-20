
ENTRY(__start)

OUTPUT_FORMAT(pei-i386)

SECTIONS
{

  . = SIZEOF_HEADERS;
  . = ALIGN(32);

  .boot . + __image_base__ :
  {
     *(.boot)
     *(.start)
     . = ALIGN(4096);
  }

  .flat . + 0xE0000000:
  {
     *(.flat) *(.text) *(.rdata) *(.data)
  }
  __edata = . - 0xE0000000;

  .bss  ALIGN(4096) :
  {
    *(.bss) *(COMMON)
  }
  __kernel_end = . - 0xE0000000;

  /DISCARD/ :
  {
    *(.debug$S)
    *(.debug$T)
    *(.debug$F)
    *(.drectve)
    *(.reloc)
    *(.edata)
  }
}



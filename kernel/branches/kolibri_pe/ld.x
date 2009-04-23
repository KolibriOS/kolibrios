
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

  .text . + 0xDFC00000:
  {
     *(.text) *(.rdata)
     . = ALIGN(4096);
  }

  .flat . + 0x00400000:
  {
     *(.flat)  *(.data)
     . = ALIGN(4096);
  }

  .edata :
  {
    *(.edata)
    _code_end = .;
    . = ALIGN(16);
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
  }
}




ENTRY(__start)

OUTPUT_FORMAT(pei-i386)

SECTIONS
{

  . = SIZEOF_HEADERS;
  . = ALIGN(32);

  .boot . + __image_base__ :
  {
     *(.boot)
     *(.init)
     . = ALIGN(4096);
  }

  .flat :
  {
     *(.flat)
  }
  __edata = .;

  .bss  ALIGN(4096) :
  {
    *(.bss) *(COMMON)
  }

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



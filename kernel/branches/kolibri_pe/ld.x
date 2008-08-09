
ENTRY(__start)

OUTPUT_FORMAT(pei-i386)

SECTIONS
{

  . = SIZEOF_HEADERS;
  . = ALIGN(32);

  .flat . + __image_base__ :
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



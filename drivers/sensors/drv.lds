

OUTPUT_FORMAT(pei-i386)

ENTRY("_drvEntry")

SECTIONS
{
  . = SIZEOF_HEADERS;
  . = ALIGN(__section_alignment__);

  .text  __image_base__ + ( __section_alignment__ < 0x1000 ? . : __section_alignment__ ) :

  {
    *(.text) *(.rdata)
  }

  .data ALIGN(__section_alignment__) :
  {
    *(.data)
  }

  .bss ALIGN(__section_alignment__):
  {
    *(.bss)
    *(COMMON)
  }

  /DISCARD/ :
  {
    *(.debug$S)
    *(.debug$T)
    *(.debug$F)
    *(.drectve)
    *(.edata)
    *(.eh_frame)
  }

  .idata ALIGN(__section_alignment__):
  {
    SORT(*)(.idata$2)
    SORT(*)(.idata$3)
    /* These zeroes mark the end of the import list. */
    LONG (0); LONG (0); LONG (0); LONG (0); LONG (0);
    SORT(*)(.idata$4)
    SORT(*)(.idata$5)
    SORT(*)(.idata$6)
    SORT(*)(.idata$7)
  }

  .reloc ALIGN(__section_alignment__) :
  {
    *(.reloc)
  }

}


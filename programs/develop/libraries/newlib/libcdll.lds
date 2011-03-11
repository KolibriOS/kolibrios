
OUTPUT_FORMAT(pei-i386)

ENTRY("_crt_startup")

SECTIONS
{

  . = SIZEOF_HEADERS;
  . = ALIGN(__section_alignment__);

  .text  __image_base__ + . :
  {
    *(.text) *(.rdata)
    . = ALIGN(16);
    ___RUNTIME_PSEUDO_RELOC_LIST__ = .;
    __RUNTIME_PSEUDO_RELOC_LIST__ = .;
    *(.rdata_runtime_pseudo_reloc)
    ___RUNTIME_PSEUDO_RELOC_LIST_END__ = .;
    __RUNTIME_PSEUDO_RELOC_LIST_END__ = .;
    __pei386_runtime_relocator = .;
  }

  .data ALIGN(__section_alignment__):
  {
     PROVIDE ( __data_start__ = .) ;
    *(.data)
        *(.data2)
        *(SORT(.data$*))
        *(.jcr)
       PROVIDE ( __data_end__ = .) ;
        *(.data_cygwin_nocopy)
  }

  .bss ALIGN(__section_alignment__):
  {
    *(.bss)
    *(COMMON)
  }

  .edata ALIGN(__section_alignment__):
  {
    *(.edata)
  }

  /DISCARD/ :
  {
    *(.debug$S)
    *(.debug$T)
    *(.debug$F)
    *(.drectve)
    *(.note.GNU-stack)
    *(.comment)
    *(.debug_abbrev)
    *(.debug_info)
    *(.debug_line)
    *(.debug_frame)
    *(.debug_loc)
    *(.debug_pubnames)
    *(.debug_aranges)
    *(.debug_ranges)
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


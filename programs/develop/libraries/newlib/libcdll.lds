
OUTPUT_FORMAT(pei-i386)

ENTRY("_crt_startup")

SECTIONS
{

  . = SIZEOF_HEADERS;
  . = ALIGN(__section_alignment__);

  .text  __image_base__ + . :
  {
    *(.text)
    *(SORT(.text$*))
    *(.text.*)
        *(.glue_7t)
        *(.glue_7)
        ___CTOR_LIST__ = .; __CTOR_LIST__ = . ;
                        LONG (-1);*(.ctors); *(.ctor); *(SORT(.ctors.*));  LONG (0);
        ___DTOR_LIST__ = .; __DTOR_LIST__ = . ;
                        LONG (-1); *(.dtors); *(.dtor); *(SORT(.dtors.*));  LONG (0);
        *(.fini)
    *(.rdata)
    *(SORT(.rdata$*))
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
     __CRT_MT = .;
     LONG(0);
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
    *(.eh_frame)
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

  .reloc ALIGN(__section_alignment__) :
  {
    *(.reloc)
  }

}


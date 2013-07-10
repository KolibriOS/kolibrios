
OUTPUT_FORMAT(pei-i386)

ENTRY("_libc_crt_startup")

SECTIONS
{

  . = SIZEOF_HEADERS;
  . = ALIGN(__section_alignment__);

  .text  __image_base__ + . :
  {
    *(.text)
    *(SORT(.text$*))
    *(.text.*)
     *(.gnu.linkonce.t.*)
        *(.glue_7t)
        *(.glue_7)
        ___CTOR_LIST__ = .; __CTOR_LIST__ = . ;
                        LONG (-1);*(.ctors); *(.ctor); *(SORT(.ctors.*));  LONG (0);
        ___DTOR_LIST__ = .; __DTOR_LIST__ = . ;
                        LONG (-1); *(.dtors); *(.dtor); *(SORT(.dtors.*));  LONG (0);
        *(.fini)
    *(.rdata)
    *(SORT(.rdata$*))
    __rt_psrelocs_start = .;
    *(.rdata_runtime_pseudo_reloc)
    __rt_psrelocs_end = .;
    __rt_psrelocs_size = __rt_psrelocs_end - __rt_psrelocs_start;
    ___RUNTIME_PSEUDO_RELOC_LIST_END__ = .;
    __RUNTIME_PSEUDO_RELOC_LIST_END__ = .;
    ___RUNTIME_PSEUDO_RELOC_LIST__ = . - __rt_psrelocs_size;
    __RUNTIME_PSEUDO_RELOC_LIST__ = . - __rt_psrelocs_size;
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
    __bss_start__ = . ;
    *(.bss)
    *(COMMON)
    __bss_end__ = . ;
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


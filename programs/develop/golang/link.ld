ENTRY(start)
SECTIONS
{
  .text 0x0000000: {
    *(.text)
   }
.rdata ALIGN(16) :
    {
        *(.rdata)
        *(SORT(.rdata$*))
        ___RUNTIME_PSEUDO_RELOC_LIST__ = .;
        __RUNTIME_PSEUDO_RELOC_LIST__ = .;
        *(.rdata_runtime_pseudo_reloc)
        ___RUNTIME_PSEUDO_RELOC_LIST_END__ = .;
        __RUNTIME_PSEUDO_RELOC_LIST_END__ = .;
    }

  .data : {
    *(.data)
   }
   
  .bss : {
    *(.bss)
   }


	end = .;
}

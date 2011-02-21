OUTPUT_FORMAT("coff-go32")
SECTIONS
{
 .text 0x000000 : 
 {
  code = . ;
  *(.text)
  *(.fixup)
  *(.gnu.warning)
  *(.gnu.linkonce.t*)
  *(.const)
  *(.ro*)
  *(.gnu.linkonce.r*)
  ecode = . ; _ecode = . ;
 }
 .data : 
 {
  djgpp_first_ctor = . ;
  *(.ctor)
  djgpp_last_ctor = . ;
  djgpp_first_dtor = . ;
  *(.dtor)
  djgpp_last_dtor = . ;
  *(.gcc_exc*)
  __EH_FRAME_BEGIN__ = . ;
  *(.eh_fram*)
  __EH_FRAME_END__ = . ;
  LONG(0);
  *(.gnu.linkonce.d*)
  *(.rodata)
  *(.rodata.*)
  *(.data)
  edata = . ; _edata = . ;
  bss = . ;
 }
 .bss :
 {
  *(.bss)
 }
 end = . ; _end = . ;
}

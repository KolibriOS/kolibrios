/*OUTPUT_FORMAT("binary")*/
ENTRY("start")
SECTIONS
{
 .text 0x000000 : 
 {
  code = . ; _code = . ;
  *(.text)
  *(.fixup)
  *(.gnu.warning)
  *(.gnu.linkonce.t*)
  *(.const)
  *(.ro*)
  *(.gnu.linkonce.r*)
  ecode = . ; _ecode = . ;
 }
 .rdata :
 {
 }
 .data : 
 {
  djgpp_first_ctor = . ;
  *(.ctor)
  *(.ctors)
  djgpp_last_ctor = . ;
  djgpp_first_dtor = . ;
  *(.dtor)
  *(.dtors)
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
 *(.note.gnu.build-id);
 }
 end = . ; _end = . ;

}


 

/*OUTPUT_FORMAT("binary")*/
ENTRY(Start)
SECTIONS
{
  .text 0x000000:
   {
    *(.text)
   }
  
  .data : {
    *(.data)
    hEnd = . ;
   }
   
  .bss :
   {
    *(.bss)
   }
   Memory = . ;
   /DISCARD/ : {
    *(.comment)
    *(.drectve)
   }
}

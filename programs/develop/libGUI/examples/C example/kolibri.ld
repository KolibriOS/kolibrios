/*OUTPUT_FORMAT("binary")*/
ENTRY("start_")
SECTIONS
{
  .text 0x000000:
   {
    *(.text)
   }
  
  .data : {
    *(.data)
   }
   
  .bss :
   {
    *(.bss)
   }
}

SECTIONS
{
  . = ALIGN(65536);
  .data.aligned :
  {
    *(SORT_BY_NAME(.data.align*))
  }

}
INSERT AFTER .data;

SECTIONS
{
  . = ALIGN(65536);
  .bss.aligned :
  {
    *(SORT_BY_NAME(.bss.align*))
  }
}
INSERT AFTER .bss;

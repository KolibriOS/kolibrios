/*
 * SPDX-FileCopyrightText: 2025 iyzsong@envs.net
 *
 * SPDX-License-Identifier: MPL-2.0
 */

SECTIONS
{
  .text 0x00000000 :
  {
    /* MENUET01 header */
    LONG(0x554e454d);
    LONG(0x31305445);
    LONG(1);
    LONG(_start);
    LONG(_image_end);
    LONG(_memory_end);
    LONG(_stack_top);
    LONG(_cmdline);
    LONG(0);
    *(.text)
    *(.text.*)
  }
  .rodata : ALIGN(8)
  {
    *(.rodata)
    *(.rodata.*)
  }
  .data : ALIGN(8)
  {
    *(.data)
  }
  _image_end = .;

  .bss : ALIGN(8)
  {
    *(.bss)
    . = . + 4K;
    _stack_top = .;
  }
  _memory_end = .;
}

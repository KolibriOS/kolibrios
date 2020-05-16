fasm __lib__.asm
fasm con_write_asciiz.asm
fasm con_write_string.asm
fasm con_init.asm
fasm con_exit.asm
fasm con_set_title.asm
fasm con_printf.asm
fasm con_get_flags.asm
fasm con_set_flags.asm
fasm con_get_font_height.asm
fasm con_get_cursor_height.asm
fasm con_getch.asm
fasm con_getch2.asm
fasm con_kbhit.asm
fasm con_gets.asm
fasm con_gets2.asm
fasm con_cls.asm
fasm con_get_cursor_pos.asm
fasm con_set_cursor_pos.asm
kos32-ar -ru libconsole.a *.o
del *.o
pause
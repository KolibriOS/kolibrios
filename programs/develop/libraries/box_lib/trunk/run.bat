@fasm.exe  -m 16384 box_lib.asm
@fasm.exe -m 16384 editbox_ex.asm editbox_ex.kex
REM ..\FASM\kpack.exe editbox_ex
@klbrinwin.exe editbox_ex.kex
@REM compiling examples
@echo off
if not exist bin mkdir bin

@copy reload_16x16_8b.png bin\reload_16x16_8b.png

@fasm.exe  -m 16384 ctrldemo.asm bin\ctrldemo.kex
@fasm.exe  -m 16384 editbox_ex.asm bin\editbox_ex.kex
@fasm.exe  -m 16384 tooltip_demo.asm bin\tooltip_demo.kex

@pause
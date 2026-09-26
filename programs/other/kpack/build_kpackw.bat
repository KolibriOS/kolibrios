@echo off
rem Build the native Windows port of kpack (no window, drag-and-drop compressor).
rem Requires FASM in PATH (or edit the line below to point at fasm.exe).
fasm -m 16384 kpackw.asm kpackw.exe

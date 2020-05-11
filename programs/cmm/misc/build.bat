@echo off

echo #define LANG_ENG 1 >lang.h--

For /R %%i In (*.c) Do c-- "%%i"

mkdir bin
del bin\*.* /Q
move *.com bin

cd bin
forfiles /S /M *.com /C "cmd /c rename @file @fname"
cd ..

del warning.txt
del lang.h--

pause
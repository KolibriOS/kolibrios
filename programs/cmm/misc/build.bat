@echo off

For /R %%i In (*.c) Do c-- /D=LANG_ENG "%%i"

mkdir bin
del bin\*.* /Q
move *.com bin

cd bin
forfiles /S /M *.com /C "cmd /c rename @file @fname"
rename software_widget syspanel
rename reshare @reshare
cd ..

del warning.txt

rem pause
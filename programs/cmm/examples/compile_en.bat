@del *.kex

For /R %%i In (*.c) Do c-- /D=LANG_ENG "%%i"

@rename *.com *.kex
@mkdir bin
@move *.kex bin\

@del warning.txt

@pause
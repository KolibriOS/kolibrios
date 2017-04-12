@del *.kex

For /R %%i In (*.c) Do c-- "%%i"

@rename *.com *.kex
@mkdir bin
@move *.kex bin\

@del warning.txt

@pause
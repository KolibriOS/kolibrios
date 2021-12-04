@del *.
@For /R %%i In (*.c) Do c-- "%%i"
@rename *.com *.
@pause
@del warning.txt

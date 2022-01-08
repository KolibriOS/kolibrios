@del *.
@For /R %%i In (*.c) Do c-- "%%i"
@rename *.com *.
if not exist imgedit ( @pause )

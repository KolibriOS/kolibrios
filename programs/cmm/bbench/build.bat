@del *.
@..\c--\c--.exe bbench.c
@rename *.com *.
@del warning.txt
if not exist bbench ( @pause )

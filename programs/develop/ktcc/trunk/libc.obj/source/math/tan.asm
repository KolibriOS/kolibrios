format COFF
section '.text'

public _tan as "tan"

_tan:
    fld qword[esp+4]
    fptan
    fxch
    ret

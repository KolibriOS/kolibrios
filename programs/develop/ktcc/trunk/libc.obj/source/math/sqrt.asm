format COFF
section '.text'

public _sqrt as "sqrt"

_sqrt:
    fld qword[esp+4]
    fsqrt
    ret

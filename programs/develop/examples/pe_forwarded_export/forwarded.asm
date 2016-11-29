format PE console 0.8 DLL at 420000h
include '../../../proc32.inc'
include '../../../import.inc'
include '../../../export.inc'

forward_by_name:
        invoke  con_write_asciiz, forward_by_name_msg
        ret

forward_by_ordinal:
        invoke  con_write_asciiz, forward_by_ordinal_msg
        ret

forward_by_name_msg     db      'Hello from forward_by_name!',13,10,0
forward_by_ordinal_msg  db      'Hello from forward_by_ordinal!',13,10,0

align 4
data import
library console, 'console.dll'
import console, con_write_asciiz, 'con_write_asciiz'
end data

align 4
data export
export 'forwarded.dll', \
        forward_by_name, 'forward_by_name', \
        forward_by_ordinal, 'forward_by_ordinal'
end data

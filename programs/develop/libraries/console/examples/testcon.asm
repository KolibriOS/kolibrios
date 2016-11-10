format PE console 0.8
include '../../../../import.inc'

start:
        push    aHelloWorld
        call    [con_write_asciiz]
        push    0
        call    [con_exit]
        xor     eax, eax
        ret

aHelloWorld  db 'Hello, World!',10,0

align 4
data import
library console, 'console.dll'
import  console,        \
        con_write_asciiz,       'con_write_asciiz',     \
        con_exit,       'con_exit'
end data

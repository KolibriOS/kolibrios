use32
        db      'MENUET01'
        dd      1
        dd      start
        dd      i_end
        dd      mem
        dd      mem
        dd      0
        dd      0

; useful includes
include '../../../../macros.inc'
purge mov,add,sub
include '../../../../proc32.inc'
include '../../../../dll.inc'

start:
; First 3 steps are intended to load/init console DLL
; and are identical for all console programs

; load DLL
        stdcall dll.Load, @IMPORT
        test    eax, eax
        jnz     exit

; yes! Now do some work (say helloworld in this case).
        push    caption
        push    -1
        push    -1
        push    -1
        push    -1
        call    [con_init]
        push    aHelloWorld
        call    [con_write_asciiz]
        push    0
        call    [con_exit]
exit:
        or      eax, -1
        int     0x40

caption      db 'Console test',0
aHelloWorld  db 'Hello, World!',10,0

align 4
@IMPORT:
library console, 'console.obj'
import  console,        \
        con_start,      'START',        \
        con_init,       'con_init',     \
        con_write_asciiz,       'con_write_asciiz',     \
        con_exit,       'con_exit',     \
        con_gets,       'con_gets'
i_end:


align 4
rb 2048 ; stack
mem:

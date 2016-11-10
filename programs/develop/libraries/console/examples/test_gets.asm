format PE console 0.8
include 'proc32.inc'
include '../../../../import.inc'

start:
        invoke  con_set_title, caption

; C-equivalent of the following code:
; for (;;)
; {
;   con_write_asciiz("Enter string (empty for exit): ");
;   if (!con_gets(s,256)) break;
;   if (s[0] == '\n') break;
;   con_write_asciiz("You entered: ");
;   con_write_asciiz(s);
; }
mainloop:
        push    str1
        call    [con_write_asciiz]
        push    256
        push    s
        call    [con_gets]
        test    eax, eax
        jz      done
        cmp     [s], 10
        jz      done
        push    str2
        call    [con_write_asciiz]
        push    s
        call    [con_write_asciiz]
        jmp     mainloop
done:
        push    1
        call    [con_exit]
exit:
        xor     eax, eax
        ret


align 4
data import
library console, 'console.dll'
import console, \
        con_set_title, 'con_set_title', \
        con_write_asciiz, 'con_write_asciiz', \
        con_exit, 'con_exit', \
        con_gets, 'con_gets'
end data

caption            db 'Console test - gets()',0
str1               db 'Enter string (empty for exit): ',0
str2               db 'You entered: ',0

s rb 256

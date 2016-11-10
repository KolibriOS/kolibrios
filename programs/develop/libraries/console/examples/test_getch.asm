format PE console 0.8
include 'proc32.inc'
include '../../../../import.inc'

start:
        invoke  con_set_title, caption

; C-equivalent of the following code:
; con_printf(start_string);
; int c;
; while ((c=con_getch())!=27) // Esc=exit
; {
;   if (c)
;     con_printf("normal character with code %d=0x%02X\n",c,c);
;   else
;   {
;     c=con_getch();
;     con_printf("extended character with code %d=0x%02X\n",c,c);
;   }
; }
        push    start_string
        call    [con_printf]
        pop     ecx
mainloop:
        call    [con_getch]
        cmp     al, 27
        jz      done
        test    eax, eax
        jz      extended
        push    eax
        push    eax
        push    string_normal
@@:
        call    [con_printf]
        add     esp, 12
        jmp     mainloop
extended:
        call    [con_getch]
        test    eax, eax
        jz      done
        push    eax
        push    eax
        push    string_extended
        jmp     @b
done:
        push    1
        call    [con_exit]
exit:
	xor	eax, eax
	ret

align 4
data import
library console, 'console.dll'
import console, \
        con_set_title, 'con_set_title', \
        con_printf, 'con_printf', \
        con_exit, 'con_exit', \
        con_getch, 'con_getch'
end data

caption            db 'Console test - getch()',0
start_string       db 'Press any key to see its code, or Esc to exit',10,0
string_normal      db 'normal character with code %d=0x%02X',10,0
string_extended    db 'extended character with code %d=0x%02X',10,0

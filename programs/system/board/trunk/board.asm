;------------------------------------------------------------------------------
; DEBUG BOARD for APPLICATIONS and KERNEL DEVELOPMENT
; See f63
; Compile with FASM for KolibriOS
;------------------------------------------------------------------------------
use32
org 0
        db      'MENUET01'
        dd      1
        dd      START
        dd      I_END
        dd      mem
        dd      mem
        dd      filename
        dd      0
;------------------------------------------------------------------------------
include 'lang.inc'
include '../../../macros.inc'
include '../../../debug.inc'
purge   newline
;SMALL FONT
MAXSTRINGS = 45
LINE_H = 10
WINDOW_W = 399
WINDOW_H = MAXSTRINGS*LINE_H+45
FONT_TYPE = 0x40000000
;BIG FONT
; MAXSTRINGS = 30
; LINE_H = 15
; WINDOW_W = 630
; WINDOW_H = MAXSTRINGS*LINE_H+50
; FONT_TYPE = 0x50000000

;------------------------------------------------------------------------------
START:
        call    CheckUnique
        mov     edi, filename
        cmp     [edi], byte 0
        jnz     param
        mov     esi, default_filename
@@:
        lodsb
        stosb
        test    al,al
        jnz     @b
param:
        mov     ecx, (MAXSTRINGS+1)*20
        mov     edi, text1
        mov     eax, '    '
        rep     stosd

        mov     ecx, (MAXSTRINGS+1)*20
        mov     edi, text2
        rep     stosd

        mov     byte [tmp1], 'x'
        mov     byte [tmp2], 'x'

        mcall   14
        and     eax, 0xffff0000
        sub     eax, WINDOW_W shl 16
        add     eax, WINDOW_W
        mov     [xstart], eax
        mcall   48, 3, sc, sizeof.system_colors

        mov     esi, filename
        call    CreateFile
;------------------------------------------------------------------------------
red:
        call    draw_window
;------------------------------------------------------------------------------
still:
        cmp     [buffer_length], 0
        je      @f
        call    write_buffer
@@:
        mcall   23, 50                    ; wait here for event
        cmp     eax, 1                    ; redraw request ?
        je      red

        cmp     eax, 2                    ; key in buffer ?
        je      key

        cmp     eax, 3                    ; button in buffer ?
        je      button

        mcall   63, 2
        cmp     ebx, 1
        jne     still

new_data:
        cmp     [buffer_length], 255
        jne     @f
        call    write_buffer
@@:
        movzx   ebx, byte[buffer_length]
        mov     [ebx+tmp], al
        inc     [buffer_length]
        mov     ebp, [targ]
        cmp     al, 10
        jz      new_line
        cmp     al, 13
        jz      new_check
        jmp     char
;------------------------------------------
write_buffer:
        pusha
        mov     edx, tmp
        movzx   ecx, byte[buffer_length]
        mov     esi, filename
.write_to_logfile:
        call    WriteToFile
        cmp     eax, 5
        jne     @f
        mov     esi, filename
        mov     [filepos], 0
        call    CreateFile
        jnc     .write_to_logfile
@@:
        movzx   eax,byte[buffer_length]
        add     [filepos],eax
        xor     eax,eax
        mov     [buffer_length],al
        popa
        ret
;------------------------------------------
new_line:
        and     [ebp-8], dword 0
        inc     dword [ebp-4]
        cmp     [ebp-4], dword MAXSTRINGS
        jbe     .noypos
        mov     [ebp-4], dword MAXSTRINGS
        lea     esi, [ebp+80]
        mov     edi, ebp
        mov     ecx, 20*(MAXSTRINGS)
        cld
        rep     movsd

        mov     esi, [ebp-4]
        imul    esi, 80
        add     esi, [ebp-8]
        add     esi, ebp
        mov     ecx, 20
        mov     eax, '    '
        rep     stosd
.noypos:
        mov     [targ],text2
        and     [krnl_cnt],0
        jmp     new_check
;------------------------------------------
char:
        cmp     ebp, text1
        je      add2
        mov     ecx, [krnl_cnt]
        cmp     al, [krnl_msg+ecx]
        jne     .noknl
        inc     [krnl_cnt]
        cmp     [krnl_cnt], 4
        jne     new_check
        mov     [targ], text1
.noknl:
        mov     ebp, [targ]
        jecxz   .add
        push    eax
        mov     esi, krnl_msg
.l1:
        lodsb
        call    add_char
        loop    .l1
        pop     eax
.add:
        and     [krnl_cnt], 0
add2:
        call    add_char

new_check:
        mcall   63, 2
        cmp     ebx, 1
        je      new_data
        call    draw_text
        jmp     still
;------------------------------------------------------------------------------
key:
        mcall   2
        cmp     ah, ' '
        je      button.noclose
        cmp     ah, 51 ; F2
        je      open_boardlog
        jmp     still
open_boardlog:
		mcall 70, open_log_in_tinypad
        jmp     red
		
;------------------------------------------------------------------------------
button:
        mcall   17                        ; get id
        cmp     ah, 1                     ; button id=1 ?
        jne     .noclose
        or      eax, -1                   ; close this program
        mcall
.noclose:
        xor     [vmode], 1
        jmp     red
;------------------------------------------------------------------------------
add_char:
        push    esi
        mov     esi, [ebp-4]
        imul    esi, 80
        add     esi, [ebp-8]
        mov     [ebp+esi], al
        inc     dword[ebp-8]
        cmp     dword[ebp-8], 80
        jb      .ok
        mov     dword[ebp-8], 79
.ok:
        pop     esi
        ret

;------------------------------------------------------------------------------
;************************  WINDOW DEFINITIONS AND DRAW ************************
;------------------------------------------------------------------------------
draw_window:
        mcall   12, 1                     ; 1, start of draw
        mcall   48, 5                     ; GetClientTop
        shr     ebx, 16
        mov     ecx, ebx
        shl     ecx, 16
        add     ecx, WINDOW_H     ; [y start] *65536 + [y size]
        mov     edx, 0xffffff
        or      edx, 0x14000000
        xor     esi, esi
        mcall   0,[xstart],,,,title
        mov     ebx, 296 shl 16+31
        mcall   8,,<4,13>,3,[sc.work_button]
        mov     edx, [vmode]
        lea     edx, [edx*4+duk]
        mov     ecx, 0x80
        shr     ecx, 24
        add     ecx, [sc.work_button_text]
        mcall   4,<300,7>,,,4
        call    draw_text
        mcall   12, 2                     ; 2, end of draw
        ret
;------------------------------------------------------------------------------
draw_text:
        mov     ebx, 15*65536+30          ; draw info text with function 4
        xor     ecx, ecx
        or      ecx, FONT_TYPE
        mov     edi, 0xffffff
        mov     edx, text1
        cmp     [vmode], 0
        je      .kern
        mov     edx, text2
.kern:
        push    ebx ecx edx
        mcall   9, procinfo,-1
        mov     eax, [ebx+42]
        xor     edx, edx
        mov     ebx, 6
        div     ebx
        pop     edx ecx ebx
        mov     esi, 80
        cmp     eax, esi
        ja      @f
        mov     esi, eax
@@:
        cmp     esi, 5
        ja      @f
        mov     esi, 5
@@:
        sub     esi, 4
        mov     eax, 4
newline:
        mcall
        add     ebx, LINE_H
        add     edx, 80
        cmp     [edx], byte 'x'
        jne     newline
        ret

;------------------------------------------------------------------------------
;*  input:  esi = pointer to the file name  *
;------------------------------------------------------------------------------
CreateFile:
        pusha
        mov     dword [InfoStructure+00], 2   ; create file
        mov     dword [InfoStructure+04], 0   ; reserved
        mov     dword [InfoStructure+08], 0   ; reserved
        mov     dword [InfoStructure+12], 0   ; 0 bytes to write (just create)
        mov     dword [InfoStructure+16], 0   ; NULL data pointer (no data)
        mov     dword [InfoStructure+20], 0   ; reserved
        mov     dword [InfoStructure+21], esi ; pointer to the file name
        mcall   70, InfoStructure
        test    eax, eax
        jz      .out
        stc
.out:
        popa
        ret
;------------------------------------------------------------------------------
;*  input:  esi = pointer to the file name  *
;*          edx = pointer to data buffer    *
;*          ecx = data length               *
;------------------------------------------------------------------------------
WriteToFile:
        push    ebx
        mov     dword [InfoStructure+00], 3   ; write to file
        mov     eax,  [filepos]
        mov     dword [InfoStructure+04], eax ; lower position addr
        mov     dword [InfoStructure+08], 0   ; upper position addr (0 for FAT)
        mov     dword [InfoStructure+12], ecx ; number of bytes to write
        mov     dword [InfoStructure+16], edx ; pointer to data buffer
        mov     dword [InfoStructure+20], 0   ; reserved
        mov     dword [InfoStructure+21], esi ; pointer to the file name
        mcall   70, InfoStructure
        clc
        test    eax, eax
        jz      .out
        stc
.out:
        pop     ebx
        ret

;------------------------------------------------------------------------------
;*  input:  esi = pointer to string         *
;*          edi = pointer to string         *
;*          ecx = data length               *
;------------------------------------------------------------------------------
StrCmp:
        repe    cmpsb
        ja      .a_greater_b
        jb      .a_less_b
.equal:
        mov     eax, 0
        jmp     .end
.a_less_b:
        mov     eax, 1
        jmp     .end
.a_greater_b:
        mov     eax, -1
.end:
        ret

;------------------------------------------------------------------------------
;*  input:  edi = pointer to string          *
;*          ecx = data length                *
;------------------------------------------------------------------------------
; 'a' - 'A' = 32 -> 'A'|32 = 'a'
ToLower:
        xor     eax, eax
.cycle:
        or      byte[edi+eax], 32
        inc     eax
        loop    .cycle
.end:
        ret

;------------------------------------------------------------------------------
;* get info on current thread, save pid/tid
;* look for another process with same name and different pid/tid
;* if found, close self
;* else continue normally
;------------------------------------------------------------------------------
CheckUnique:
.get_thread_info:
        mov     ebx, procinfo
        mov     ecx, -1
        mcall   9

.get_pid:                             ; check_buffer
        mov     [process_count], eax
        mov     eax, [ebx+process_information.PID]
        mov     [pid_tid], eax
        mov     ecx, 2

.check_threads:
        cmp     ecx, [process_count]
        ja      .leave_check
        mov     eax, 9
        mcall

.check_slot_free:
        cmp     dword [ebx+process_information.slot_state], 9
        je      .next_thread

.check_pid:
        mov     eax, [pid_tid]
        cmp     [ebx+process_information.PID], eax
        je      .next_thread

.get_proc_name:
        lea     edi, [ebx+process_information.process_name]
        push    ecx
        mov     ecx, my_name_size-1

.lower_case:
        call    ToLower
        lea     esi, [my_name]
        mov     ecx, my_name_size
        call    StrCmp
        pop     ecx
        cmp     eax, 0
        je      .close_program

.next_thread:
        inc     ecx
        jmp     .check_threads

.close_program:
        ; restore and active window of previous thread
        mcall   18, 3
        mov     eax, -1
        mcall

.leave_check:
        ret

;------------------------------------------------------------------------------
; DATA

if lang eq ru
 title	db 'Доска отладки и сообщений',0
else if lang eq it
 title	db 'Notifiche e informazioni generiche per il debug',0
else if lang eq ge
 title	db 'Allgemeines debug- & nachrichtenboard',0
else
 title	db 'General debug & message board',0
end if

default_filename db '/tmp0/1/boardlog.txt',0
krnl_msg        db  'K : '
duk             db  'KernUser'
my_name         db  'board',0
my_name_size = $-my_name

align 4
vmode   dd  1
targ    dd  text2

I_END:

InfoStructure:
        dd      ?       ; subfunction number
        dd      ?       ; position in the file in bytes
        dd      ?       ; upper part of the position address
        dd      ?       ; number of bytes to read
        dd      ?       ; pointer to the buffer to write data
        db      ?
        dd      ?       ; pointer to the filename

open_log_in_tinypad:
        dd      7
        dd      0
        dd      filename
        dd      0
        dd      0
        db      '/sys/develop/cedit',0

buffer_length   rb  3
process_count   dd  ?
krnl_cnt        dd  ?
pid_tid         dd  ?
filepos         dd  ?
xstart          dd  ?
sc      system_colors

        rd  2
text1   rb  80*(MAXSTRINGS+1)
tmp1    dd  ?

        rd  2
text2   rb  80*(MAXSTRINGS+1)
tmp2    dd  ?

tmp             rb  256
filename        rb  256
procinfo        rb  1024
stackbuf        rb  2000h
mem:
